cmake_minimum_required(VERSION 3.14)
project(VoiceAssistDesktop VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build test programs" OFF)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/voice_recognizer.cpp
    src/llm_client.cpp
    src/audio_manager.cpp
    src/main.cpp
)

# Define the executable
add_executable(voice_assist_desktop ${SOURCES})

# Link libraries
target_link_libraries(voice_assist_desktop PRIVATE 
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Installation
install(TARGETS voice_assist_desktop
        RUNTIME DESTINATION bin
)

# Add pkg-config dependencies based on platform
if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PULSE REQUIRED libpulse)
    pkg_check_modules(PULSE_SIMPLE REQUIRED libpulse-simple)
    target_include_directories(voice_assist_desktop PRIVATE ${PULSE_INCLUDE_DIRS} ${PULSE_SIMPLE_INCLUDE_DIRS})
    target_link_libraries(voice_assist_desktop PRIVATE ${PULSE_LIBRARIES} ${PULSE_SIMPLE_LIBRARIES})
elseif(APPLE)
    # macOS-specific libraries
    find_library(CORE_AUDIO CoreAudio)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    target_link_libraries(voice_assist_desktop PRIVATE ${CORE_AUDIO} ${AUDIO_TOOLBOX})
elseif(WIN32)
    # Windows-specific libraries
    target_link_libraries(voice_assist_desktop PRIVATE winmm)
endif()

# Tests configuration
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif() 