# Voice Assistant Application

A cross-platform voice assistant application that accepts voice input, interacts with LLM APIs, and provides voiced or text responses.

## Features

- **Voice Input Recognition**: Captures and processes user's speech input
- **LLM API Integration**: Sends transcribed text to a language model API (e.g., OpenAI GPT)
- **Interactive UI**: Clean, modern interface with conversation history
- **Context Awareness**: Maintains conversation context for follow-up questions
- **Error Handling**: Robust error management for speech recognition and API issues
- **Cross-Platform Support**: Android mobile app and desktop applications (Windows, macOS, Linux)
- **Text-to-Speech**: Converts assistant responses to spoken audio

## Project Structure

The project follows a standard architecture with separate Android and C++ components:

### Android Application (Kotlin)

```
app/
├── src/
│   ├── main/
│   │   ├── java/com/voiceassist/
│   │   │   ├── api/             # API service interfaces and models
│   │   │   ├── model/           # Data models 
│   │   │   ├── ui/              # UI components like adapters
│   │   │   ├── viewmodel/       # ViewModels for business logic
│   │   │   └── MainActivity.kt  # Main application activity
│   │   ├── res/
│   │   │   ├── drawable/        # Icons and graphics
│   │   │   ├── layout/          # UI layout files
│   │   │   ├── menu/            # Menu definitions
│   │   │   └── values/          # Strings, colors, and dimensions
│   │   └── AndroidManifest.xml  # App manifest
│   └── build.gradle             # Module build configuration
└── build.gradle                 # Project build configuration
```

### Desktop Application (C++)

```
cpp/
├── include/                     # Header files
│   ├── audio_manager.h          # Audio recording/playback
│   ├── voice_recognizer.h       # Speech-to-text functionality
│   ├── llm_client.h             # LLM API client
│   └── voice_assistant.h        # Main application class
├── src/                         # Implementation files
│   ├── audio_manager.cpp        # Platform-specific audio implementations
│   ├── voice_recognizer.cpp     # Platform-specific recognition implementations
│   ├── llm_client.cpp           # API interaction implementation
│   └── main.cpp                 # Command-line interface
└── CMakeLists.txt               # CMake build configuration
```

## Cross-Platform Integration

The project is designed to support cross-platform functionality in several ways:

1. **Shared Business Logic**: Core functionality for LLM interaction and conversation management follows similar patterns across platforms.

2. **JNI Integration Option**: The C++ components can be exposed to the Android app through JNI (Java Native Interface) to reuse platform-specific audio processing or custom ML models.

3. **Potential for Kotlin Multiplatform**: In the future, the project could evolve to use Kotlin Multiplatform to share more code between Android and desktop versions.

4. **Shared Configuration**: Both platforms use similar configuration structures for the LLM settings, voice recognition parameters, and TTS options.

## Setup Instructions

### Android Application

1. Clone the repository
2. Open the project in Android Studio
3. Add your LLM API key in `LlmRepository.kt`
4. Build and run the application

### Desktop Application

1. Install dependencies:
   - CMake (3.14 or higher)
   - libcurl-dev
   - nlohmann-json (will be fetched if not found)
   - Platform-specific audio libraries:
     - Windows: WinMM
     - macOS: CoreAudio and AudioToolbox
     - Linux: PulseAudio

2. Build the application:
```bash
cd cpp
mkdir build
cd build
cmake ..
make
```

3. Run the application:
```bash
# Set API key in environment
export LLM_API_KEY=your_api_key_here

# Run the application
./voice_assist_desktop
```

## Technology Stack

### Android App
- **Language**: Kotlin
- **Architecture**: MVVM (Model-View-ViewModel)
- **UI Framework**: Android's Material Design components
- **Voice Input**: Android SpeechRecognizer API
- **Networking**: Retrofit for API communication
- **Concurrency**: Kotlin Coroutines for asynchronous operations

### Desktop App
- **Language**: C++17
- **Architecture**: Component-based design with platform abstraction
- **Networking**: libcurl for HTTP requests
- **JSON Processing**: nlohmann/json
- **Concurrency**: C++ std::thread and std::future
- **Voice Recognition**: Platform-specific APIs
- **Audio**: Platform-specific audio libraries

## Requirements

### Android App
- Android 6.0 (API level 23) or higher
- Internet connection for LLM API access
- Microphone permission

### Desktop App
- Operating System: Windows, macOS, or Linux
- Internet connection for LLM API access
- Microphone for voice input
- Speakers for audio output

## Future Enhancements

- Multi-language support
- Custom voice selection for TTS
- GUI for desktop application
- Local history storage
- Settings customization
- Offline mode for basic functionality
- Shared code between mobile and desktop via Kotlin Multiplatform
- Unified build system for both platforms

## License

[MIT License](LICENSE) 