#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

// Hailo NPU voice input pipeline: silero VAD + Hailo Speech2Text (Whisper-Tiny.hef).
// Compiled only when VRENGINE_HAS_HAILO=1.
class VoiceInputThread final : private juce::AudioIODeviceCallback
{
public:
    using TranscriptCallback = std::function<void (const juce::String&)>;

    VoiceInputThread() = default;
    ~VoiceInputThread();

    // Opens the microphone and starts the pipeline.
    // onTranscript is called on the JUCE message thread for each utterance.
    // Returns false if audio device init fails.
    bool start (TranscriptCallback onTranscript);
    void stop();
    bool isRunning() const noexcept { return running.load(); }

private:
    void audioDeviceAboutToStart (juce::AudioIODevice*) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext (const float* const*, int,
                                           float* const*, int, int,
                                           const juce::AudioIODeviceCallbackContext&) override;

    void workerLoop (TranscriptCallback callback, std::string hefPath);
    static std::vector<float> resampleToWhisperRate (const std::vector<float>& input, double inputRate);

    juce::AudioDeviceManager audioDeviceManager;
    std::atomic<bool>        running   { false };
    std::atomic<bool>        shouldStop { false };
    std::thread              worker;
    std::mutex               mutex;
    std::condition_variable  cv;
    std::vector<float>       micBuffer;
    double                   micSampleRate { 16000.0 };
};
