#ifndef VOICE_ASSISTANT_H
#define VOICE_ASSISTANT_H

#include "audio_manager.h"
#include "voice_recognizer.h"
#include "llm_client.h"

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <mutex>

namespace voice_assist {

/**
 * @brief Configuration for the voice assistant
 */
struct VoiceAssistantConfig {
    std::string apiKey;
    std::string llmModel = "gpt-3.5-turbo";
    std::string language = "en-US";
    std::string ttsVoice = "";
    bool useTextToSpeech = true;
    bool saveConversationHistory = true;
    int maxContextMessages = 10;
};

/**
 * @brief Main class for the voice assistant application
 * 
 * Coordinates audio recording, voice recognition, LLM API communication,
 * and text-to-speech playback.
 */
class VoiceAssistant {
public:
    enum class State {
        IDLE,
        LISTENING,
        PROCESSING,
        RESPONDING
    };
    
    using StateChangeCallback = std::function<void(State)>;
    using TranscriptionCallback = std::function<void(const std::string&)>;
    using ResponseCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    
    VoiceAssistant(const VoiceAssistantConfig& config = VoiceAssistantConfig());
    ~VoiceAssistant();
    
    /**
     * @brief Initializes the voice assistant
     * @return true if initialization was successful
     */
    bool initialize();
    
    /**
     * @brief Starts listening for voice input
     */
    bool startListening();
    
    /**
     * @brief Stops listening
     */
    void stopListening();
    
    /**
     * @brief Sends a text input directly to the assistant
     */
    void sendTextInput(const std::string& text);
    
    /**
     * @brief Gets the current state
     */
    State getState() const;
    
    /**
     * @brief Clears the conversation history
     */
    void clearConversation();
    
    /**
     * @brief Sets configuration options
     */
    void setConfig(const VoiceAssistantConfig& config);
    
    /**
     * @brief Gets current configuration
     */
    const VoiceAssistantConfig& getConfig() const;
    
    /**
     * @brief Sets the state change callback
     */
    void setStateChangeCallback(StateChangeCallback callback);
    
    /**
     * @brief Sets the transcription callback
     */
    void setTranscriptionCallback(TranscriptionCallback callback);
    
    /**
     * @brief Sets the response callback
     */
    void setResponseCallback(ResponseCallback callback);
    
    /**
     * @brief Sets the error callback
     */
    void setErrorCallback(ErrorCallback callback);
    
    /**
     * @brief Gets the conversation history
     */
    std::vector<Message> getConversationHistory() const;

private:
    std::unique_ptr<AudioManager> audioManager_;
    std::unique_ptr<VoiceRecognizer> voiceRecognizer_;
    std::unique_ptr<LlmClient> llmClient_;
    
    VoiceAssistantConfig config_;
    State state_ = State::IDLE;
    std::vector<Message> conversationHistory_;
    std::mutex mutex_;
    
    StateChangeCallback stateChangeCallback_;
    TranscriptionCallback transcriptionCallback_;
    ResponseCallback responseCallback_;
    ErrorCallback errorCallback_;
    
    void setState(State state);
    void handleTranscription(const std::string& text);
    void handleLlmResponse(const std::string& response);
    void reportError(const std::string& error);
};

} // namespace voice_assist

#endif // VOICE_ASSISTANT_H 