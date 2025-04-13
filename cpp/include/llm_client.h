#ifndef LLM_CLIENT_H
#define LLM_CLIENT_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <future>

namespace voice_assist {

/**
 * @brief Represents a single message in a conversation
 */
struct Message {
    enum class Role {
        SYSTEM,
        USER,
        ASSISTANT
    };

    Role role;
    std::string content;
    std::string id;
    
    Message(Role role, const std::string& content);
};

/**
 * @brief Configuration for the LLM client
 */
struct LlmClientConfig {
    std::string apiKey;
    std::string baseUrl = "https://api.openai.com/v1/";
    std::string model = "gpt-3.5-turbo";
    float temperature = 0.7f;
    int maxTokens = 150;
    int timeout = 30; // seconds
};

/**
 * @brief Client for interacting with Language Model APIs
 */
class LlmClient {
public:
    using ResponseCallback = std::function<void(const std::string&, bool)>;
    
    LlmClient(const LlmClientConfig& config = LlmClientConfig());
    ~LlmClient();
    
    /**
     * @brief Sends a conversation to the LLM API and returns the response
     * 
     * @param messages The conversation history
     * @param callback Function to call with the response or error
     * @return std::future<std::string> Future containing the response
     */
    std::future<std::string> sendConversation(
        const std::vector<Message>& messages,
        ResponseCallback callback = nullptr
    );
    
    /**
     * @brief Sets the API configuration
     */
    void setConfig(const LlmClientConfig& config);
    
    /**
     * @brief Gets the current configuration
     */
    const LlmClientConfig& getConfig() const;
    
    /**
     * @brief Cancels any pending requests
     */
    void cancelPendingRequests();

private:
    LlmClientConfig config_;
    bool cancelRequested_ = false;
    std::mutex mutex_;
    
    std::string buildRequestBody(const std::vector<Message>& messages);
    std::string performRequest(const std::string& endpoint, const std::string& body);
    std::string parseResponse(const std::string& jsonResponse);
};

} // namespace voice_assist

#endif // LLM_CLIENT_H 