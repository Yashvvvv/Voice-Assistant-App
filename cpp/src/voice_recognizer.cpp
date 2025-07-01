#include "voice_recognizer.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <sapi.h>
#include <sphelper.h>
#include <atlbase.h>
#include <thread>
#include <string>
#elif __APPLE__
    // macOS-specific includes will go here
#else
    // Linux-specific includes will go here
#endif

namespace voice_assist {

VoiceRecognizer::VoiceRecognizer(const VoiceRecognizerConfig& config)
    : config_(config), listening_(false) {
}

VoiceRecognizer::~VoiceRecognizer() {
    if (listening_) {
        stopListening();
    }
}

bool VoiceRecognizer::isListening() const {
    return listening_;
}

void VoiceRecognizer::setConfig(const VoiceRecognizerConfig& config) {
    config_ = config;
}

const VoiceRecognizerConfig& VoiceRecognizer::getConfig() const {
    return config_;
}

#ifdef _WIN32
// Windows implementation

class WindowsVoiceRecognizer : public VoiceRecognizer {
public:
    WindowsVoiceRecognizer(const VoiceRecognizerConfig& config)
        : VoiceRecognizer(config),
          recognizer_(nullptr),
          reco_context_(nullptr),
          grammar_(nullptr),
          com_initialized_(false),
          retry_count_(0),
          max_retry_count_(3) {
        // Windows-specific initialization
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(hr)) {
            com_initialized_ = true;
            std::cout << "COM initialized successfully" << std::endl;
        } else {
            std::cerr << "Failed to initialize COM: " << std::hex << hr << std::endl;
        }
    }

    ~WindowsVoiceRecognizer() override {
        stopListening();
        if (com_initialized_) {
            CoUninitialize();
        }
    }

    bool startListening(TranscriptionCallback callback) override {
        if (listening_) {
            return false;
        }

        callback_ = std::move(callback);

        // Reset retry counter when starting fresh
        retry_count_ = 0;

        return initializeRecognizer();
    }

    void stopListening() override {
        if (!listening_) {
            return;
        }

        listening_ = false;
        if (listener_thread_.joinable()) {
            listener_thread_.join();
        }

        cleanupRecognizer();

        std::cout << "Windows voice recognition stopped" << std::endl;
    }

private:
    bool initializeRecognizer() {
        cleanupRecognizer();

        HRESULT hr = recognizer_.CoCreateInstance(CLSID_SpInprocRecognizer);
        if (FAILED(hr)) {
            std::cerr << "Failed to create recognizer instance: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to create recognizer instance");
        }

        CComPtr<ISpAudio> audio;
        hr = SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOIN, &audio);
        if (FAILED(hr)) {
            std::cerr << "Failed to create audio input: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to create audio input");
        }

        hr = recognizer_->SetInput(audio, TRUE);
        if (FAILED(hr)) {
            std::cerr << "Failed to set audio input: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to set audio input");
        }

        hr = recognizer_->CreateRecoContext(&reco_context_);
        if (FAILED(hr)) {
            std::cerr << "Failed to create recognition context: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to create recognition context");
        }

        hr = reco_context_->SetNotifyWin32Event();
        if (FAILED(hr)) {
            std::cerr << "Failed to set notify event: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to set notify event");
        }

        h_event_ = reco_context_->GetNotifyEventHandle();
        if (h_event_ == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to get notify event handle" << std::endl;
            return tryRecovery("Failed to get notify event handle");
        }

        hr = reco_context_->CreateGrammar(0, &grammar_);
        if (FAILED(hr)) {
            std::cerr << "Failed to create grammar: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to create grammar");
        }

        hr = grammar_->LoadDictation(nullptr, SPLO_STATIC);
        if (FAILED(hr)) {
            std::cerr << "Failed to load dictation: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to load dictation");
        }

        hr = grammar_->SetDictationState(SPRS_ACTIVE);
        if (FAILED(hr)) {
            std::cerr << "Failed to activate dictation: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to activate dictation");
        }

        hr = recognizer_->SetRecoState(SPRST_ACTIVE);
        if (FAILED(hr)) {
            std::cerr << "Failed to set recognizer state: " << std::hex << hr << std::endl;
            return tryRecovery("Failed to set recognizer state");
        }

        listening_ = true;
        listener_thread_ = std::thread(&WindowsVoiceRecognizer::listenLoop, this);

        std::cout << "Windows voice recognition started" << std::endl;
        return true;
    }

    bool tryRecovery(const std::string& errorMsg) {
        cleanupRecognizer();

        if (retry_count_ < max_retry_count_) {
            retry_count_++;
            std::cerr << "Retrying speech recognition setup (attempt " << retry_count_
                      << " of " << max_retry_count_ << ")" << std::endl;

            // Small delay before retry
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return initializeRecognizer();
        } else {
            std::cerr << "Error: " << errorMsg << " - Max retries exceeded" << std::endl;
            return false;
        }
    }

    void cleanupRecognizer() {
        if (recognizer_) {
            recognizer_->SetRecoState(SPRST_INACTIVE);
        }
        if (grammar_) {
            grammar_.Release();
        }
        if (reco_context_) {
            reco_context_->SetNotifySink(nullptr);
            reco_context_.Release();
        }
        if (recognizer_) {
            recognizer_.Release();
        }
    }

    void listenLoop() {
        while (listening_) {
            DWORD wait_result = WaitForSingleObject(h_event_, 250); // Check every 250ms

            if (!listening_) break;

            if (wait_result == WAIT_TIMEOUT) {
                continue;
            } else if (wait_result != WAIT_OBJECT_0) {
                std::cerr << "Error waiting for recognition events: " << GetLastError() << std::endl;
                break;
            }

            CSpEvent event;
            while (reco_context_ && SUCCEEDED(event.GetFrom(reco_context_)) && event.eEventId != SPEI_FALSE_RECOGNITION) {
                if (event.eEventId == SPEI_RECOGNITION) {
                    const auto* phrase = event.RecoResult();
                    if (phrase) {
                        wchar_t* text = nullptr;
                        if (SUCCEEDED(phrase->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &text, nullptr))) {
                            std::wstring wstr(text);
                            CoTaskMemFree(text);
                            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
                            std::string str(size_needed, 0);
                            WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
                            if (callback_) {
                                callback_(str);
                            }
                        }
                    }
                } else if (event.eEventId == SPEI_END_SR_STREAM) {
                    // Recognition stream has ended
                    if (listening_) {
                        // If we're still supposed to be listening, restart recognition
                        std::cout << "Recognition stream ended, restarting..." << std::endl;
                        initializeRecognizer();
                    }
                }
            }
        }
    }

    CComPtr<ISpRecognizer> recognizer_;
    CComPtr<ISpRecoContext> reco_context_;
    CComPtr<ISpRecoGrammar> grammar_;
    HANDLE h_event_ = INVALID_HANDLE_VALUE;
    std::thread listener_thread_;
    bool com_initialized_;
    int retry_count_;
    int max_retry_count_;
};

#elif __APPLE__
// macOS implementation

class MacOSVoiceRecognizer : public VoiceRecognizer {
public:
    MacOSVoiceRecognizer(const VoiceRecognizerConfig& config) 
        : VoiceRecognizer(config) {
        // macOS-specific initialization
    }
    
    ~MacOSVoiceRecognizer() override {
        stopListening();
    }
    
    bool startListening(TranscriptionCallback callback) override {
        if (listening_) {
            return false;
        }
        
        // Store callback
        callback_ = std::move(callback);
        
        // TODO: Implement macOS speech recognition using
        // NSSpeechRecognizer or similar
        
        std::cout << "macOS voice recognition started" << std::endl;
        listening_ = true;
        
        // For now, this is a placeholder implementation
        
        return true;
    }
    
    void stopListening() override {
        if (!listening_) {
            return;
        }
        
        // TODO: Stop macOS speech recognition
        
        std::cout << "macOS voice recognition stopped" << std::endl;
        listening_ = false;
    }
    
private:
    // macOS-specific members
};

#else
// Linux implementation

class LinuxVoiceRecognizer : public VoiceRecognizer {
public:
    LinuxVoiceRecognizer(const VoiceRecognizerConfig& config) 
        : VoiceRecognizer(config) {
        // Linux-specific initialization
    }
    
    ~LinuxVoiceRecognizer() override {
        stopListening();
    }
    
    bool startListening(TranscriptionCallback callback) override {
        if (listening_) {
            return false;
        }
        
        // Store callback
        callback_ = std::move(callback);
        
        // TODO: Implement Linux speech recognition using
        // PocketSphinx, Mozilla DeepSpeech, or similar
        
        std::cout << "Linux voice recognition started" << std::endl;
        listening_ = true;
        
        // For now, this is a placeholder implementation
        
        return true;
    }
    
    void stopListening() override {
        if (!listening_) {
            return;
        }
        
        // TODO: Stop Linux speech recognition
        
        std::cout << "Linux voice recognition stopped" << std::endl;
        listening_ = false;
    }
    
private:
    // Linux-specific members
};

#endif

std::unique_ptr<VoiceRecognizer> createVoiceRecognizer(const VoiceRecognizerConfig& config) {
#ifdef _WIN32
    return std::make_unique<WindowsVoiceRecognizer>(config);
#elif __APPLE__
    return std::make_unique<MacOSVoiceRecognizer>(config);
#else
    return std::make_unique<LinuxVoiceRecognizer>(config);
#endif
}

} // namespace voice_assist
