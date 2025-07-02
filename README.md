# Voice Assistant Application

A cross-platform voice assistant application that accepts voice input, interacts with Large Language Model (LLM) APIs, and provides voiced or text responses.

## Features

-   **Voice Input Recognition**: Captures and processes user's speech input using Android's SpeechRecognizer.
-   **LLM API Integration**: Sends transcribed text to a generative language model API (e.g., Google Gemini) and processes responses.
-   **Interactive UI**: Clean, modern interface with real-time conversation history.
-   **Context Awareness**: Maintains conversation context for follow-up questions for a more natural interaction.
-   **Robust Error Handling**: Comprehensive error management for speech recognition and LLM API communication.
-   **Cross-Platform Potential**: Designed with a structure that supports an Android mobile app and potential for desktop applications (Windows, macOS, Linux) with shared logic.
-   **Text-to-Speech**: (Planned/Future Enhancement, though not fully implemented in the current core Android flow, it's mentioned in original features).

## Project Structure

The project follows a modular architecture, separating Android (Kotlin) and native C++ components.

### Android Application (Kotlin)

```
app/
├── src/
│   ├── main/
│   │   ├── java/com/voiceassist/
│   │   │   ├── api/             # Retrofit interfaces and data models for LLM API interaction (LlmService, LlmData)
│   │   │   ├── model/           # Data models for conversation messages (Message, Sender)
│   │   │   ├── ui/              # UI components like RecyclerView adapters (MessageAdapter)
│   │   │   ├── viewmodel/       # ViewModels for managing UI-related data and business logic (MainViewModel)
│   │   │   └── MainActivity.kt  # Main application activity, handles UI interactions and orchestrates components
│   │   ├── res/
│   │   │   ├── drawable/        # Vector assets and custom drawables
│   │   │   ├── layout/          # UI layout definitions (activity_main.xml, item_message.xml)
│   │   │   ├── menu/            # Menu definitions
│   │   │   └── values/          # Strings, colors, and dimensions
│   │   └── AndroidManifest.xml  # App manifest, declares permissions and components
│   └── build.gradle             # Module-specific build configuration for Android app
└── build.gradle                 # Project-level build configuration
```

### Native C++ Components

```
cpp/
├── include/                     # Header files for C++ modules
│   ├── audio_manager.h          # Handles audio recording/playback operations
│   ├── voice_recognizer.h       # Interfaces for speech-to-text functionality
│   ├── llm_client.h             # Client for direct LLM API interaction (if used natively)
│   └── voice_assistant.h        # Main interface for native voice assistant logic
├── src/                         # Implementation files for C++ modules
│   ├── audio_manager.cpp        # Platform-specific audio implementations
│   ├── voice_recognizer.cpp     # Platform-specific recognition implementations
│   ├── llm_client.cpp           # LLM API interaction implementation
│   └── main.cpp                 # Entry point for a standalone command-line application (if desktop)
└── CMakeLists.txt               # CMake build configuration for native components
```

## Android Application Architecture

The Android application follows the **Model-View-ViewModel (MVVM)** architectural pattern to ensure a clear separation of concerns, testability, and maintainability.

*   **View (MainActivity, Layouts):** The UI layer responsible for displaying data and handling user interactions. It observes changes in the ViewModel.
*   **ViewModel (MainViewModel):** Acts as a bridge between the View and the Model. It holds UI-related data, handles user input, and interacts with the Repository to fetch or process data. It exposes `LiveData` to the View.
*   **Model (Message, Sender, API Data Models):** Represents the data and business logic. This includes:
    *   **Data Models:** `Message`, `Sender`, `GenerateContentRequest`, `GenerateContentResponse`, etc.
    *   **Repository (LlmRepository):** Abstracts data sources, providing a clean API to the ViewModel. It decides whether to fetch data from a network service or other sources.
    *   **Service (LlmService):** Defines the network API calls using Retrofit.

## Control Flow (Android Application)

This diagram illustrates how user voice input flows through the application and triggers responses from the LLM.

```mermaid
graph TD
    A[User Voice Input] --> B{MainActivity};
    B -- Check Permission --> C[Android SpeechRecognizer];
    C -- Recognized Text --> B;
    B -- Process Input --> D[MainViewModel];
    D -- Update UI State<br>(isListening, isProcessing) --> B;
    D -- Add User Message<br>Add Placeholder --> E[Conversation History (LiveData)];
    E --> F[MessageAdapter];
    F --> G[RecyclerView (UI)];
    D -- Request LLM Response --> H[LlmRepository];
    H -- Call LLM API --> I[LlmService];
    I -- HTTP Request --> J[LLM API Endpoint];
    J -- HTTP Response --> I;
    I -- Parsed Response --> H;
    H -- Result<String> --> D;
    D -- Update Assistant Message<br>Clear Placeholder --> E;
    D -- Update UI State<br>(isProcessing false) --> B;
    B --> G;
    D -- Error Event --> B;
    B -- Show Error (Snackbar) --> User;
```

## Data Flow (Android Application)

This diagram shows the movement of data between different components of the Android application.

```mermaid
graph LR
    subgraph UI Layer
        MainActivity
        MessageAdapter
        RecyclerView
    end

    subgraph ViewModel Layer
        MainViewModel
    end

    subgraph Data Layer
        LlmRepository
        LlmService
        LlmData
    end

    subgraph External Services
        Android Speech Recognition Service
        LLM API (Gemini)
    end

    UserVoiceAudio -- Audio Stream --> Android Speech Recognition Service;
    Android Speech Recognition Service -- Transcribed Text (String) --> MainActivity;
    MainActivity -- Transcribed Text (String) --> MainViewModel;
    
    MainViewModel -- User Message (Message Object) --> Conversation History (List<Message>);
    MainViewModel -- LLM Context (List<Message>) --> LlmRepository;
    
    LlmRepository -- GenerateContentRequest (JSON) --> LlmService;
    LlmService -- HTTP Request --> LLM API (Gemini);
    LLM API (Gemini) -- HTTP Response (JSON) --> LlmService;
    LlmService -- GenerateContentResponse (JSON) --> LlmRepository;
    LlmRepository -- Assistant Response (String) --> MainViewModel;
    
    MainViewModel -- Conversation History (LiveData<List<Message>>) --> MessageAdapter;
    MessageAdapter -- Message Objects --> RecyclerView (UI);
    RecyclerView -- Displays --> User;

    style Android Speech Recognition Service fill:#bbf,stroke:#333,stroke-width:2px;
    style LLM API (Gemini) fill:#bbf,stroke:#333,stroke-width:2px;
```

## Cross-Platform Integration

The project is designed with potential for cross-platform functionality:

1.  **Shared Business Logic**: Core functionality for LLM interaction and conversation management follows similar patterns across platforms, making it easier to port or reuse.
2.  **JNI Integration Option**: The C++ components (e.g., for audio processing, voice recognition, or custom LLM clients) can be exposed to the Android app through JNI (Java Native Interface) to reuse high-performance native code.
3.  **Future Kotlin Multiplatform**: The project architecture could evolve to use Kotlin Multiplatform to share more code directly between Android and desktop versions.
4.  **Shared Configuration**: Both platforms use similar configuration structures for LLM settings, voice recognition parameters, and potentially TTS options.

## Setup Instructions

### Android Application

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/your-repo/VoiceAssist.git
    cd VoiceAssist
    ```
2.  **Open in Android Studio**: Open the `VoiceAssist` project in Android Studio.
3.  **Add your LLM API Key**:
    *   Create a `local.properties` file in your project root (if it doesn't exist).
    *   Add your Gemini API key to your `gradle.properties` file:
        ```properties
        GEMINI_API_KEY="YOUR_GEMINI_API_KEY_HERE"
        ```
    *   Ensure `BuildConfig.GEMINI_API_KEY` is correctly set up in `app/build.gradle` (usually automatically picked from `gradle.properties`).
4.  **Build and Run**: Connect an Android device or use an Android emulator, then build and run the application from Android Studio.

### Desktop Application (C++)

*(Note: This part describes a potential standalone C++ desktop application, separate from the main Android app's execution flow. Its integration with the Android app is via JNI for specific functionalities.)*

1.  **Install Dependencies**:
    *   **CMake**: (3.14 or higher) for building the C++ project.
    *   **libcurl-dev**: For HTTP requests to the LLM API.
    *   **nlohmann-json**: A C++ JSON library (will be fetched if not found during CMake configuration).
    *   **Platform-specific audio libraries**:
        *   **Windows**: WinMM
        *   **macOS**: CoreAudio and AudioToolbox
        *   **Linux**: PulseAudio
2.  **Build the application**:
    ```bash
    cd cpp
    mkdir build
    cd build
    cmake ..
    make
    ```
3.  **Run the application**:
    ```bash
    # Set API key in environment
    export LLM_API_KEY=your_api_key_here

    # Run the application (executable name might vary based on CMakeLists.txt)
    ./voice_assist_desktop
    ```

## Technology Stack

### Android App
-   **Language**: Kotlin
-   **Architecture**: MVVM (Model-View-ViewModel)
-   **UI Framework**: Android Jetpack Compose and Material Design components (though `activity_main.xml` and `item_message.xml` suggest classic View system usage)
-   **Voice Input**: Android SpeechRecognizer API
-   **Networking**: Retrofit for API communication with Gson converter
-   **Concurrency**: Kotlin Coroutines for asynchronous operations
-   **Dependency Injection**: (Implicit, or could use Hilt/Koin for larger projects)

### Native C++ Components
-   **Language**: C++17
-   **Build System**: CMake
-   **Networking**: libcurl for HTTP requests
-   **JSON Processing**: nlohmann/json
-   **Concurrency**: C++ `std::thread` and `std::future`
-   **Voice Recognition/Audio**: Platform-specific APIs and audio libraries

## Requirements

### Android App
-   Android 6.0 (API level 23) or higher
-   Internet connection for LLM API access
-   Microphone permission (`RECORD_AUDIO`)

### Desktop App
-   Operating System: Windows, macOS, or Linux
-   Internet connection for LLM API access
-   Microphone for voice input
-   Speakers for audio output

## Future Enhancements

-   Multi-language support for voice recognition and LLM interaction.
-   Custom voice selection for Text-to-Speech (TTS).
-   Full-fledged Graphical User Interface (GUI) for the desktop application.
-   Local history storage and management.
-   Advanced settings customization.
-   Offline mode for basic functionality (e.g., local voice commands).
-   Enhanced shared code between mobile and desktop via Kotlin Multiplatform.
-   Unified build system for both platforms for streamlined development.
-   Integration with other LLMs or AI services.

## License

[MIT License](LICENSE) 