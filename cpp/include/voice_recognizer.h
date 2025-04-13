#ifndef VOICE_RECOGNIZER_H
#define VOICE_RECOGNIZER_H

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace voice_assist {

/**
 * @brief Configuration options for voice recognition
 */
struct VoiceRecognizerConfig {
    std::string language = "en-US";
    float speechThreshold = 0.2f;
    int maxRecordingTimeMs = 15000;
    int silenceTimeoutMs = 2000;
};

/**
 * @brief Abstract base class for voice recognition functionality
 */
class VoiceRecognizer {
public:
    using TranscriptionCallback = std::function<void(const std::string&, bool)>;
    
    VoiceRecognizer(const VoiceRecognizerConfig& config = VoiceRecognizerConfig());
    virtual ~VoiceRecognizer();

    /**
     * @brief Starts listening for voice input
     * @param callback Function to call when transcription is available
     */
    virtual bool startListening(TranscriptionCallback callback) = 0;
    
    /**
     * @brief Stops the current listening session
     */
    virtual void stopListening() = 0;
    
    /**
     * @brief Checks if the recognizer is currently listening
     */
    virtual bool isListening() const;
    
    /**
     * @brief Sets the configuration
     */
    void setConfig(const VoiceRecognizerConfig& config);
    
    /**
     * @brief Gets the current configuration
     */
    const VoiceRecognizerConfig& getConfig() const;

protected:
    VoiceRecognizerConfig config_;
    bool listening_ = false;
    TranscriptionCallback callback_;
};

/**
 * @brief Creates platform-specific voice recognizer instance
 */
std::unique_ptr<VoiceRecognizer> createVoiceRecognizer(const VoiceRecognizerConfig& config = VoiceRecognizerConfig());

} // namespace voice_assist

#endif // VOICE_RECOGNIZER_H 