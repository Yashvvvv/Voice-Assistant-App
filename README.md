# Voice Assistant Application

A cross-platform voice assistant application that accepts voice input, interacts with LLM APIs, and provides voiced or text responses.

## Features

- **Voice Input Recognition**: Captures and processes user's speech input
- **LLM API Integration**: Sends transcribed text to a language model API (e.g., OpenAI GPT)
- **Interactive UI**: Clean, modern interface with conversation history
- **Context Awareness**: Maintains conversation context for follow-up questions
- **Error Handling**: Robust error management for speech recognition and API issues

## Project Structure

The project follows a standard MVVM architecture:

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

## Setup Instructions

1. Clone the repository
2. Open the project in Android Studio
3. Add your LLM API key in `LlmRepository.kt`
4. Build and run the application

## Technology Stack

- **Language**: Kotlin for Android
- **Architecture**: MVVM (Model-View-ViewModel)
- **UI Framework**: Android's Material Design components
- **Voice Input**: Android SpeechRecognizer API
- **Networking**: Retrofit for API communication
- **Concurrency**: Kotlin Coroutines for asynchronous operations

## Requirements

- Android 6.0 (API level 23) or higher
- Internet connection for LLM API access
- Microphone permission

## Future Enhancements

- Multi-language support
- Custom voice selection for TTS
- Desktop application using cross-platform frameworks
- Local history storage
- Settings customization
- Offline mode for basic functionality

## License

[MIT License](LICENSE) 