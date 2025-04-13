#include "voice_assistant.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <atomic>

// Global flag for handling interrupts
std::atomic<bool> g_running(true);

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
}

void printHelp() {
    std::cout << "Voice Assistant Commands:" << std::endl;
    std::cout << "  listen      - Start listening for voice input" << std::endl;
    std::cout << "  stop        - Stop listening" << std::endl;
    std::cout << "  type TEXT   - Send text directly to the assistant" << std::endl;
    std::cout << "  clear       - Clear conversation history" << std::endl;
    std::cout << "  api KEY     - Set the API key" << std::endl;
    std::cout << "  model MODEL - Set the LLM model" << std::endl;
    std::cout << "  tts on|off  - Enable/disable text-to-speech" << std::endl;
    std::cout << "  help        - Display this help message" << std::endl;
    std::cout << "  exit        - Exit the application" << std::endl;
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    std::signal(SIGINT, signalHandler);  // Ctrl+C
    std::signal(SIGTERM, signalHandler); // Termination request
    
    // Welcome message
    std::cout << "Voice Assistant - Cross-Platform CLI" << std::endl;
    std::cout << "Type 'help' for a list of commands" << std::endl;
    
    // Check for API key in environment
    std::string apiKey = std::getenv("LLM_API_KEY") ? std::getenv("LLM_API_KEY") : "";
    if (apiKey.empty()) {
        std::cout << "Warning: No API key found. Set it with 'api YOUR_KEY' or LLM_API_KEY env var." << std::endl;
    }
    
    // Initialize configuration
    voice_assist::VoiceAssistantConfig config;
    config.apiKey = apiKey;
    
    // Create voice assistant
    std::unique_ptr<voice_assist::VoiceAssistant> assistant;
    try {
        assistant = std::make_unique<voice_assist::VoiceAssistant>(config);
        
        // Set up callbacks
        assistant->setStateChangeCallback([](voice_assist::VoiceAssistant::State state) {
            switch (state) {
                case voice_assist::VoiceAssistant::State::IDLE:
                    std::cout << "State: IDLE" << std::endl;
                    break;
                case voice_assist::VoiceAssistant::State::LISTENING:
                    std::cout << "State: LISTENING..." << std::endl;
                    break;
                case voice_assist::VoiceAssistant::State::PROCESSING:
                    std::cout << "State: PROCESSING..." << std::endl;
                    break;
                case voice_assist::VoiceAssistant::State::RESPONDING:
                    std::cout << "State: RESPONDING..." << std::endl;
                    break;
            }
        });
        
        assistant->setTranscriptionCallback([](const std::string& text) {
            std::cout << "You said: " << text << std::endl;
        });
        
        assistant->setResponseCallback([](const std::string& response) {
            std::cout << "Assistant: " << response << std::endl;
        });
        
        assistant->setErrorCallback([](const std::string& error) {
            std::cerr << "Error: " << error << std::endl;
        });
        
        // Initialize the assistant
        if (!assistant->initialize()) {
            std::cerr << "Failed to initialize voice assistant" << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        return 1;
    }
    
    // Main command loop
    std::string line;
    while (g_running) {
        // Display prompt
        std::cout << "> ";
        std::getline(std::cin, line);
        
        // Handle empty input
        if (line.empty()) {
            continue;
        }
        
        // Parse command
        std::string command = line;
        std::string arg;
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            command = line.substr(0, spacePos);
            arg = line.substr(spacePos + 1);
        }
        
        // Process command
        if (command == "help") {
            printHelp();
        } 
        else if (command == "exit") {
            g_running = false;
        } 
        else if (command == "listen") {
            if (!assistant->startListening()) {
                std::cerr << "Failed to start listening" << std::endl;
            }
        } 
        else if (command == "stop") {
            assistant->stopListening();
        } 
        else if (command == "type") {
            if (arg.empty()) {
                std::cout << "Please provide text to send" << std::endl;
            } else {
                assistant->sendTextInput(arg);
            }
        } 
        else if (command == "clear") {
            assistant->clearConversation();
            std::cout << "Conversation history cleared" << std::endl;
        } 
        else if (command == "api") {
            if (arg.empty()) {
                std::cout << "Please provide an API key" << std::endl;
            } else {
                auto cfg = assistant->getConfig();
                cfg.apiKey = arg;
                assistant->setConfig(cfg);
                std::cout << "API key set" << std::endl;
            }
        } 
        else if (command == "model") {
            if (arg.empty()) {
                std::cout << "Please provide a model name" << std::endl;
            } else {
                auto cfg = assistant->getConfig();
                cfg.llmModel = arg;
                assistant->setConfig(cfg);
                std::cout << "Model set to: " << arg << std::endl;
            }
        } 
        else if (command == "tts") {
            if (arg == "on") {
                auto cfg = assistant->getConfig();
                cfg.useTextToSpeech = true;
                assistant->setConfig(cfg);
                std::cout << "Text-to-speech enabled" << std::endl;
            } 
            else if (arg == "off") {
                auto cfg = assistant->getConfig();
                cfg.useTextToSpeech = false;
                assistant->setConfig(cfg);
                std::cout << "Text-to-speech disabled" << std::endl;
            } 
            else {
                std::cout << "Please specify 'on' or 'off'" << std::endl;
            }
        } 
        else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }
    
    std::cout << "Exiting..." << std::endl;
    return 0;
} 