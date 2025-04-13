#include "llm_client.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <thread>

namespace voice_assist {

// Helper function for CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Constructor for Message
Message::Message(Role role, const std::string& content) 
    : role(role), content(content) {
    // Generate a unique ID
    static size_t counter = 0;
    id = "msg_" + std::to_string(counter++);
}

LlmClient::LlmClient(const LlmClientConfig& config)
    : config_(config), cancelRequested_(false) {
    // Initialize curl globally (should be done only once in the application)
    curl_global_init(CURL_GLOBAL_ALL);
}

LlmClient::~LlmClient() {
    // Clean up curl
    curl_global_cleanup();
}

std::future<std::string> LlmClient::sendConversation(
    const std::vector<Message>& messages,
    ResponseCallback callback
) {
    // Reset cancel flag
    std::lock_guard<std::mutex> lock(mutex_);
    cancelRequested_ = false;
    
    // Create a promise for the result
    auto promise = std::make_shared<std::promise<std::string>>();
    
    // Launch in a separate thread
    std::thread t([this, promise, messages, callback]() {
        try {
            // Build request body
            std::string requestBody = buildRequestBody(messages);
            
            // Perform the request
            std::string response = performRequest("chat/completions", requestBody);
            
            // Parse the response
            std::string result = parseResponse(response);
            
            // Call the callback if provided
            if (callback) {
                callback(result, false);
            }
            
            // Set the promise value
            promise->set_value(result);
        } catch (const std::exception& e) {
            std::string errorMsg = "LLM API error: " + std::string(e.what());
            std::cerr << errorMsg << std::endl;
            
            // Call the callback with the error if provided
            if (callback) {
                callback(errorMsg, true);
            }
            
            // Set the promise exception
            promise->set_exception(std::make_exception_ptr(std::runtime_error(errorMsg)));
        }
    });
    
    // Detach the thread
    t.detach();
    
    // Return the future from the promise
    return promise->get_future();
}

void LlmClient::setConfig(const LlmClientConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
}

const LlmClientConfig& LlmClient::getConfig() const {
    return config_;
}

void LlmClient::cancelPendingRequests() {
    std::lock_guard<std::mutex> lock(mutex_);
    cancelRequested_ = true;
}

std::string LlmClient::buildRequestBody(const std::vector<Message>& messages) {
    using json = nlohmann::json;
    
    // Create the main request object
    json requestJson = {
        {"model", config_.model},
        {"temperature", config_.temperature},
        {"max_tokens", config_.maxTokens}
    };
    
    // Build the messages array
    json messagesJson = json::array();
    for (const auto& message : messages) {
        // Convert role enum to string
        std::string roleStr;
        switch (message.role) {
            case Message::Role::SYSTEM:
                roleStr = "system";
                break;
            case Message::Role::USER:
                roleStr = "user";
                break;
            case Message::Role::ASSISTANT:
                roleStr = "assistant";
                break;
        }
        
        // Add message to array
        messagesJson.push_back({
            {"role", roleStr},
            {"content", message.content}
        });
    }
    
    // Add messages to request
    requestJson["messages"] = messagesJson;
    
    // Convert to string
    return requestJson.dump();
}

std::string LlmClient::performRequest(const std::string& endpoint, const std::string& body) {
    // Check if canceled
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cancelRequested_) {
            throw std::runtime_error("Request canceled");
        }
    }
    
    // Initialize curl session
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    // Response string
    std::string responseString;
    
    // Full URL
    std::string url = config_.baseUrl + endpoint;
    
    // Set up the request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config_.timeout);
    
    // Set up HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + config_.apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set the request body
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    
    // Check for errors
    if (res != CURLE_OK) {
        std::string errorMsg = "CURL error: " + std::string(curl_easy_strerror(res));
        
        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        throw std::runtime_error(errorMsg);
    }
    
    // Get HTTP response code
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    // Check HTTP status
    if (httpCode < 200 || httpCode >= 300) {
        std::ostringstream errorMsg;
        errorMsg << "HTTP error " << httpCode << ": " << responseString;
        throw std::runtime_error(errorMsg.str());
    }
    
    return responseString;
}

std::string LlmClient::parseResponse(const std::string& jsonResponse) {
    try {
        // Parse JSON
        auto responseJson = nlohmann::json::parse(jsonResponse);
        
        // Extract the choice
        if (responseJson.contains("choices") && 
            responseJson["choices"].is_array() && 
            !responseJson["choices"].empty() &&
            responseJson["choices"][0].contains("message") &&
            responseJson["choices"][0]["message"].contains("content")) {
            
            return responseJson["choices"][0]["message"]["content"];
        }
        
        throw std::runtime_error("Invalid response format: " + jsonResponse);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("JSON parsing error: ") + e.what());
    }
}

} // namespace voice_assist 