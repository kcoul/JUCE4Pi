// Hailo NPU voice pipeline for VREngine.
// Silero VAD (via ONNX Runtime) detects utterance boundaries; Hailo Speech2Text transcribes.
// No whisper.cpp dependency — Hailo-only path throughout.

// Windows COM headers (included via HailoRT's platform.h) define `interface` as `struct`,
// which conflicts with HailoRT's use of `interface` as a parameter name in hailort_defaults.hpp.
// Suppress inclusion by hailort.hpp, then include manually after clearing the COM macro.
#ifdef _WIN32
#  define _HAILO_HAILORT_DEFAULTS_HPP_
#endif
#include <hailo/genai/speech2text/speech2text.hpp>
#include <hailo/hailort.hpp>
#ifdef _WIN32
#  undef _HAILO_HAILORT_DEFAULTS_HPP_
#  undef interface
#  include <hailo/hailort_defaults.hpp>
#endif

#include "SileroVad.h"
#include "VoiceInput_Hailo.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace
{
constexpr int    whisperSampleRate      = 16000;
constexpr double vadFrameSeconds        = static_cast<double> (SileroVad::kWindowSamples) / whisperSampleRate;
constexpr double vadPreRollSeconds      = 0.25;
constexpr double vadMinSpeechSeconds    = 0.18;
constexpr double vadEndSilenceSeconds   = 0.28;
constexpr double vadMaxUtteranceSeconds = 2.5;
constexpr float  vadSpeechThreshold     = 0.50f;
constexpr auto   hailoVDeviceGroupId    = "SHARED";

static std::string findHailoWhisperHef (const juce::String& modelName)
{
    const auto binary = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    const auto home   = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
    const auto name   = "Whisper-" + modelName + ".hef";

    const juce::File candidates[] = {
        binary.getParentDirectory().getChildFile ("models/hailo10h").getChildFile (name),
        home.getChildFile ("bridge/models/hailo10h").getChildFile (name),
        home.getChildFile ("bridge").getChildFile (name),
    };

    for (const auto& f : candidates)
        if (f.existsAsFile())
            return f.getFullPathName().toStdString();

    throw std::runtime_error ("Hailo Whisper HEF not found: " + name.toStdString()
                              + ". Place it next to the binary in models/hailo10h/");
}

static std::shared_ptr<hailort::VDevice> createSharedHailoVDevice()
{
    hailo_vdevice_params_t params {};
    const auto status = hailo_init_vdevice_params (&params);
    if (status != HAILO_SUCCESS)
        throw hailort::hailort_error (status, "Failed to initialize Hailo VDevice params");

    params.group_id = hailoVDeviceGroupId;

    auto vdevice = hailort::VDevice::create_shared (params);
    if (! vdevice)
        throw hailort::hailort_error (vdevice.status(), "Failed to create shared Hailo VDevice");

    return vdevice.release();
}
} // namespace

// =============================================================================

VoiceInputThread::~VoiceInputThread()
{
    stop();
}

bool VoiceInputThread::start (TranscriptCallback onTranscript, const juce::String& modelName)
{
    if (running.load())
        return true;

    std::string hefPath;
    try
    {
        hefPath = findHailoWhisperHef (modelName);
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog (juce::String ("VoiceInputThread: ") + e.what());
        return false;
    }

    const auto result = audioDeviceManager.initialise (1, 0, nullptr, true);
    if (result.isNotEmpty())
    {
        juce::Logger::writeToLog ("VoiceInputThread: audio init failed: " + result);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock (mutex);
        micBuffer.clear();
        micSampleRate = whisperSampleRate;
    }

    shouldStop.store (false);
    running.store (true);
    audioDeviceManager.addAudioCallback (this);

    worker = std::thread ([this,
                           cb = std::move (onTranscript),
                           hef = std::move (hefPath)]() mutable {
        workerLoop (std::move (cb), std::move (hef));
    });

    return true;
}

void VoiceInputThread::stop()
{
    if (! running.exchange (false))
        return;

    audioDeviceManager.removeAudioCallback (this);
    audioDeviceManager.closeAudioDevice();

    {
        std::lock_guard<std::mutex> lock (mutex);
        shouldStop.store (true);
    }
    cv.notify_all();

    if (worker.joinable())
        worker.join();
}

void VoiceInputThread::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    std::lock_guard<std::mutex> lock (mutex);
    micSampleRate = device != nullptr ? device->getCurrentSampleRate() : whisperSampleRate;
    micBuffer.clear();
}

void VoiceInputThread::audioDeviceStopped()
{
    std::lock_guard<std::mutex> lock (mutex);
    micBuffer.clear();
}

void VoiceInputThread::audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                                          int numInputChannels,
                                                          float* const* outputChannelData,
                                                          int numOutputChannels,
                                                          int numSamples,
                                                          const juce::AudioIODeviceCallbackContext&)
{
    for (int ch = 0; ch < numOutputChannels; ++ch)
        if (outputChannelData[ch] != nullptr)
            juce::FloatVectorOperations::clear (outputChannelData[ch], numSamples);

    if (! running.load() || numInputChannels <= 0 || numSamples <= 0)
        return;

    std::vector<float> mono (static_cast<size_t> (numSamples));
    for (int i = 0; i < numSamples; ++i)
    {
        float sum = 0.0f;
        int   used = 0;
        for (int ch = 0; ch < numInputChannels; ++ch)
        {
            if (inputChannelData[ch] != nullptr)
            {
                sum += inputChannelData[ch][i];
                ++used;
            }
        }
        mono[static_cast<size_t> (i)] = used > 0 ? sum / static_cast<float> (used) : 0.0f;
    }

    {
        std::lock_guard<std::mutex> lock (mutex);
        micBuffer.insert (micBuffer.end(), mono.begin(), mono.end());
    }
    cv.notify_one();
}

void VoiceInputThread::workerLoop (TranscriptCallback callback, std::string hefPath)
{
    // ── Silero VAD init ───────────────────────────────────────────────────────
    const auto vadModelFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                                  .getParentDirectory().getChildFile ("silero_vad.onnx");
    std::unique_ptr<SileroVad> vad;
    try { vad = std::make_unique<SileroVad> (vadModelFile.getFullPathName().toStdString()); }
    catch (const std::exception& e)
    {
        juce::MessageManager::callAsync ([this, msg = juce::String (e.what())] {
            juce::Logger::writeToLog ("VoiceInputThread: Silero VAD init failed: " + msg);
            running.store (false);
        });
        return;
    }

    // ── Hailo Speech2Text init ────────────────────────────────────────────────
    std::shared_ptr<hailort::VDevice>            hailoVDevice;
    std::unique_ptr<hailort::genai::Speech2Text> speech2Text;

    try
    {
        hailoVDevice = createSharedHailoVDevice();
        auto s2tParams   = hailort::genai::Speech2TextParams (hefPath);
        auto s2tExpected = hailort::genai::Speech2Text::create (hailoVDevice, s2tParams);

        if (! s2tExpected)
            throw hailort::hailort_error (s2tExpected.status(), "Failed to create Hailo Speech2Text");

        speech2Text = std::make_unique<hailort::genai::Speech2Text> (s2tExpected.release());
    }
    catch (const std::exception& e)
    {
        juce::MessageManager::callAsync ([this, msg = juce::String (e.what())] {
            juce::Logger::writeToLog ("VoiceInputThread: Hailo init failed: " + msg);
            running.store (false);
        });
        return;
    }

    { std::lock_guard<std::mutex> lock (mutex); micBuffer.clear(); }

    const auto preRollSamples    = static_cast<size_t> (vadPreRollSeconds    * whisperSampleRate);
    const auto minSpeechSamples  = static_cast<size_t> (vadMinSpeechSeconds  * whisperSampleRate);
    const auto endSilenceSamples = static_cast<size_t> (vadEndSilenceSeconds * whisperSampleRate);
    const auto maxUtterSamples   = static_cast<size_t> (vadMaxUtteranceSeconds * whisperSampleRate);

    std::vector<float> preRoll, utterance;
    bool   speechActive  = false;
    size_t silenceSamples = 0;

    // ── Main VAD loop ─────────────────────────────────────────────────────────
    while (! shouldStop.load())
    {
        std::vector<float> chunk;
        double sampleRate = whisperSampleRate;

        {
            std::unique_lock<std::mutex> lock (mutex);
            cv.wait (lock, [this] {
                const auto needed = static_cast<size_t> (micSampleRate * vadFrameSeconds);
                return shouldStop.load() || micBuffer.size() >= needed;
            });

            if (shouldStop.load())
                break;

            sampleRate = micSampleRate;
            const auto needed = static_cast<size_t> (sampleRate * vadFrameSeconds);
            if (micBuffer.size() < needed)
                continue;

            // Drop backlog when not in speech to avoid processing stale audio.
            if (! speechActive && micBuffer.size() > needed * 20)
                micBuffer.erase (micBuffer.begin(), micBuffer.end() - static_cast<std::ptrdiff_t> (needed * 20));

            chunk.assign (micBuffer.begin(), micBuffer.begin() + static_cast<std::ptrdiff_t> (needed));
            micBuffer.erase (micBuffer.begin(), micBuffer.begin() + static_cast<std::ptrdiff_t> (needed));
        }

        auto pcm16k = resampleToWhisperRate (chunk, sampleRate);
        if (pcm16k.empty())
            continue;

        // Pad to Silero window size if resampling gave slightly fewer samples.
        if (pcm16k.size() < static_cast<size_t> (SileroVad::kWindowSamples))
            pcm16k.resize (static_cast<size_t> (SileroVad::kWindowSamples), 0.0f);

        const float prob          = vad->predict (pcm16k.data());
        const bool  frameHasSpeech = (prob >= vadSpeechThreshold);

        if (! speechActive)
        {
            preRoll.insert (preRoll.end(), pcm16k.begin(), pcm16k.end());
            if (preRoll.size() > preRollSamples)
                preRoll.erase (preRoll.begin(), preRoll.end() - static_cast<std::ptrdiff_t> (preRollSamples));

            if (! frameHasSpeech)
                continue;

            speechActive  = true;
            silenceSamples = 0;
            utterance     = preRoll;
        }
        else
        {
            utterance.insert (utterance.end(), pcm16k.begin(), pcm16k.end());
        }

        if (frameHasSpeech) silenceSamples = 0;
        else                 silenceSamples += pcm16k.size();

        const bool hasEnoughSpeech   = utterance.size() >= minSpeechSamples;
        const bool reachedEndSilence  = silenceSamples  >= endSilenceSamples;
        const bool reachedMaxDuration = utterance.size() >= maxUtterSamples;

        if (! reachedMaxDuration && (! hasEnoughSpeech || ! reachedEndSilence))
            continue;

        // ── Transcribe ───────────────────────────────────────────────────────
        try
        {
            auto genParams = speech2Text->create_generator_params()
                                 .expect ("Failed to create Hailo Speech2Text generator params");

            auto status = genParams.set_task (hailort::genai::Speech2TextTask::TRANSCRIBE);
            if (status != HAILO_SUCCESS)
                throw hailort::hailort_error (status, "set_task failed");

            status = genParams.set_language ("en");
            if (status != HAILO_SUCCESS)
                throw hailort::hailort_error (status, "set_language failed");

            const auto textResult = speech2Text->generate_all_text (
                hailort::MemoryView (const_cast<float*> (utterance.data()),
                                     utterance.size() * sizeof (float)),
                genParams,
                std::chrono::milliseconds (15000))
                    .expect ("Hailo transcription failed");

            juce::String transcript = juce::String (textResult).trim();
            if (transcript.isNotEmpty())
                juce::MessageManager::callAsync ([cb = callback, text = std::move (transcript)] {
                    cb (text);
                });
        }
        catch (const std::exception& e)
        {
            juce::MessageManager::callAsync ([this, msg = juce::String (e.what())] {
                juce::Logger::writeToLog ("VoiceInputThread: transcription error: " + msg);
                running.store (false);
            });
            break;
        }

        speechActive  = false;
        silenceSamples = 0;
        utterance.clear();
        preRoll.clear();
        vad->reset();
    }
}

std::vector<float> VoiceInputThread::resampleToWhisperRate (const std::vector<float>& input,
                                                              double inputRate)
{
    if (input.empty())
        return {};

    if (std::abs (inputRate - whisperSampleRate) < 1.0)
        return input;

    const auto outputSize = static_cast<size_t> (
        std::max (1.0, std::floor ((static_cast<double> (input.size()) * whisperSampleRate) / inputRate)));

    std::vector<float> output (outputSize);
    const auto ratio = inputRate / static_cast<double> (whisperSampleRate);

    for (size_t i = 0; i < output.size(); ++i)
    {
        const auto sourcePos = static_cast<double> (i) * ratio;
        const auto index     = static_cast<size_t> (sourcePos);
        const auto frac      = static_cast<float> (sourcePos - static_cast<double> (index));
        const auto a = input[std::min (index,     input.size() - 1)];
        const auto b = input[std::min (index + 1, input.size() - 1)];
        output[i] = a + (b - a) * frac;
    }

    return output;
}
