#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>

namespace voice_assist {

/**
 * @brief Audio format specification
 */
struct AudioFormat {
    int sampleRate = 16000;
    int channels = 1;
    int bitsPerSample = 16;
};

/**
 * @brief Configuration for audio recording
 */
struct AudioConfig {
    AudioFormat format;
    float gainLevel = 1.0f;
    bool echoCancellation = true;
    bool noiseSuppression = true;
    int bufferSizeMs = 100;
};

/**
 * @brief Abstract base class for cross-platform audio recording and playback
 */
class AudioManager {
public:
    using AudioSampleCallback = std::function<void(const std::vector<int16_t>&, bool)>;
    
    AudioManager(const AudioConfig& config = AudioConfig());
    virtual ~AudioManager();
    
    /**
     * @brief Starts audio recording
     * 
     * @param callback Function to call with audio samples
     * @return bool Success or failure
     */
    virtual bool startRecording(AudioSampleCallback callback) = 0;
    
    /**
     * @brief Stops audio recording
     */
    virtual void stopRecording() = 0;
    
    /**
     * @brief Checks if currently recording
     */
    virtual bool isRecording() const;
    
    /**
     * @brief Plays back audio data
     * 
     * @param audioData Vector of audio samples to play
     * @param format Format of the audio data
     * @return bool Success or failure
     */
    virtual bool playAudio(const std::vector<int16_t>& audioData, const AudioFormat& format = AudioFormat()) = 0;
    
    /**
     * @brief Converts text to speech and plays it
     * 
     * @param text The text to speak
     * @param voice The voice to use (implementation-dependent)
     * @return bool Success or failure
     */
    virtual bool speak(const std::string& text, const std::string& voice = "") = 0;
    
    /**
     * @brief Sets the audio configuration
     */
    void setConfig(const AudioConfig& config);
    
    /**
     * @brief Gets the current configuration
     */
    const AudioConfig& getConfig() const;

protected:
    AudioConfig config_;
    bool recording_ = false;
    AudioSampleCallback callback_;
};

/**
 * @brief Creates a platform-specific audio manager instance
 */
std::unique_ptr<AudioManager> createAudioManager(const AudioConfig& config = AudioConfig());

} // namespace voice_assist

#endif // AUDIO_MANAGER_H 