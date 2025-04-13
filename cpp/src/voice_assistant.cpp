#include "voice_assistant.h"
#include <iostream>
#include <algorithm>

namespace voice_assist {

VoiceAssistant::VoiceAssistant(const VoiceAssistantConfig& config)
    : config_(config), state_(State::IDLE) {
}

VoiceAssistant::~VoiceAssistant() {
    // Stop any ongoing operations
    if (state_ == State::LISTENING) {
        stopListening();
    }
}

bool VoiceAssistant::initialize() {
    try {
        // Create the audio manager
        audioManager_ = createAudioManager();
        if (!audioManager_) {
            reportError("Failed to create audio manager");
            return false;
        }
        
        // Create the voice recognizer
        VoiceRecognizerConfig voiceConfig;
        voiceConfig.language = config_.language;
        voiceRecognizer_ = createVoiceRecognizer(voiceConfig);
        if (!voiceRecognizer_) {
            reportError("Failed to create voice recognizer");
            return false;
        }
        
        // Create the LLM client
        LlmClientConfig llmConfig;
        llmConfig.apiKey = config_.apiKey;
        llmConfig.model = config_.llmModel;
        llmClient_ = std::make_unique<LlmClient>(llmConfig);
        
        // Add a system message to start the conversation
        conversationHistory_.push_back(Message(
            Message::Role::SYSTEM,
            "You are a helpful assistant providing concise and accurate information."
        ));
        
        setState(State::IDLE);
        return true;
    }
    catch (const std::exception& e) {
        reportError(std::string("Initialization error: ") + e.what());
        return false;
    }
}

bool VoiceAssistant::startListening() {
    if (state_ != State::IDLE) {
        reportError("Cannot start listening in current state");
        return false;
    }
    
    // Set up the audio sample callback
    if (!audioManager_->startRecording([this](const std::vector<int16_t>& audioData, bool isFinal) {
        // This is where audio processing would happen in a real implementation
    })) {
        reportError("Failed to start audio recording");
        return false;
    }
    
    // Set up the transcription callback
    if (!voiceRecognizer_->startListening([this](const std::string& text, bool isFinal) {
        // This callback is invoked when transcription is available
        if (isFinal && !text.empty()) {
            handleTranscription(text);
        }
    })) {
        // Clean up if voice recognition fails
        audioManager_->stopRecording();
        reportError("Failed to start voice recognition");
        return false;
    }
    
    setState(State::LISTENING);
    return true;
}

void VoiceAssistant::stopListening() {
    if (state_ != State::LISTENING) {
        return;
    }
    
    voiceRecognizer_->stopListening();
    audioManager_->stopRecording();
    
    setState(State::IDLE);
}

void VoiceAssistant::sendTextInput(const std::string& text) {
    if (state_ != State::IDLE) {
        reportError("Cannot process text input in current state");
        return;
    }
    
    // Process the text as if it was transcribed
    handleTranscription(text);
}

VoiceAssistant::State VoiceAssistant::getState() const {
    return state_;
}

void VoiceAssistant::clearConversation() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Keep only the system message
    conversationHistory_.clear();
    
    // Add a system message to start the conversation
    conversationHistory_.push_back(Message(
        Message::Role::SYSTEM,
        "You are a helpful assistant providing concise and accurate information."
    ));
}

void VoiceAssistant::setConfig(const VoiceAssistantConfig& config) {
    // Only update config if we're in an idle state
    if (state_ != State::IDLE) {
        reportError("Cannot change configuration in current state");
        return;
    }
    
    config_ = config;
    
    // Update sub-component configurations if needed
    if (llmClient_) {
        LlmClientConfig llmConfig = llmClient_->getConfig();
        llmConfig.apiKey = config_.apiKey;
        llmConfig.model = config_.llmModel;
        llmClient_->setConfig(llmConfig);
    }
    
    if (voiceRecognizer_) {
        VoiceRecognizerConfig voiceConfig = voiceRecognizer_->getConfig();
        voiceConfig.language = config_.language;
        voiceRecognizer_->setConfig(voiceConfig);
    }
}

const VoiceAssistantConfig& VoiceAssistant::getConfig() const {
    return config_;
}

void VoiceAssistant::setStateChangeCallback(StateChangeCallback callback) {
    stateChangeCallback_ = std::move(callback);
}

void VoiceAssistant::setTranscriptionCallback(TranscriptionCallback callback) {
    transcriptionCallback_ = std::move(callback);
}

void VoiceAssistant::setResponseCallback(ResponseCallback callback) {
    responseCallback_ = std::move(callback);
}

void VoiceAssistant::setErrorCallback(ErrorCallback callback) {
    errorCallback_ = std::move(callback);
}

std::vector<Message> VoiceAssistant::getConversationHistory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return conversationHistory_;
}

void VoiceAssistant::setState(State state) {
    state_ = state;
    
    // Notify callback if registered
    if (stateChangeCallback_) {
        stateChangeCallback_(state);
    }
}

void VoiceAssistant::handleTranscription(const std::string& text) {
    // Notify callback
    if (transcriptionCallback_) {
        transcriptionCallback_(text);
    }
    
    // Add to conversation history
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conversationHistory_.push_back(Message(Message::Role::USER, text));
        
        // Trim conversation history if needed
        if (config_.maxContextMessages > 0 && 
            conversationHistory_.size() > static_cast<size_t>(config_.maxContextMessages) + 1) {
            // Keep system message at index 0, remove oldest messages after that
            conversationHistory_.erase(
                conversationHistory_.begin() + 1,
                conversationHistory_.end() - config_.maxContextMessages
            );
        }
    }
    
    // Set processing state
    setState(State::PROCESSING);
    
    // Get LLM response
    std::vector<Message> currentHistory;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        currentHistory = conversationHistory_;
    }
    
    llmClient_->sendConversation(
        currentHistory,
        [this](const std::string& response, bool isError) {
            if (isError) {
                reportError(response);
                setState(State::IDLE);
            } else {
                handleLlmResponse(response);
            }
        }
    );
}

void VoiceAssistant::handleLlmResponse(const std::string& response) {
    // Add to conversation history
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conversationHistory_.push_back(Message(Message::Role::ASSISTANT, response));
    }
    
    // Notify callback
    if (responseCallback_) {
        responseCallback_(response);
    }
    
    // Set responding state
    setState(State::RESPONDING);
    
    // Text-to-speech if enabled
    if (config_.useTextToSpeech) {
        audioManager_->speak(response, config_.ttsVoice);
    }
    
    // Return to idle state
    setState(State::IDLE);
}

void VoiceAssistant::reportError(const std::string& error) {
    if (errorCallback_) {
        errorCallback_(error);
    }
}

} // namespace voice_assist 