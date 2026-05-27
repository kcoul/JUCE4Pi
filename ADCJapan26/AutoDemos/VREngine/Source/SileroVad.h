#pragma once
// Silero VAD v5 — thin ONNX Runtime wrapper.
// Feed exactly kWindowSamples of 16 kHz mono float per predict() call.
// Returns speech probability [0, 1]; call reset() between utterances.
#include <onnxruntime_cxx_api.h>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

class SileroVad
{
public:
    static constexpr int kWindowSamples = 512;  // 32 ms @ 16 kHz

    explicit SileroVad (const std::string& modelPath)
        : env_     (ORT_LOGGING_LEVEL_WARNING, "SileroVad"),
          session_ (env_, std::filesystem::path (modelPath).c_str(), Ort::SessionOptions{})
    {
        state_.assign (2 * 1 * 128, 0.0f);
    }

    SileroVad (const void* modelData, size_t modelBytes)
        : env_     (ORT_LOGGING_LEVEL_WARNING, "SileroVad"),
          session_ (env_, modelData, modelBytes, Ort::SessionOptions{})
    {
        state_.assign (2 * 1 * 128, 0.0f);
    }

    // Returns speech probability. samples must point to exactly kWindowSamples floats.
    float predict (const float* samples)
    {
        const int64_t inShape[]    = { 1, kWindowSamples };
        const int64_t srShape[]    = { 1 };
        const int64_t stateShape[] = { 2, 1, 128 };
        int64_t       sr           = 16000;

        auto mem = Ort::MemoryInfo::CreateCpu (OrtArenaAllocator, OrtMemTypeDefault);

        std::vector<Ort::Value> ins;
        ins.reserve (3);
        ins.push_back (Ort::Value::CreateTensor<float>  (mem, const_cast<float*> (samples), kWindowSamples,  inShape,    2));
        ins.push_back (Ort::Value::CreateTensor<float>  (mem, state_.data(),  state_.size(), stateShape, 3));
        ins.push_back (Ort::Value::CreateTensor<int64_t>(mem, &sr, 1,                        srShape,    1));

        const char* iNames[] = { "input", "state", "sr" };
        const char* oNames[] = { "output", "stateN" };

        auto outs = session_.Run (Ort::RunOptions{}, iNames, ins.data(), 3, oNames, 2);

        std::memcpy (state_.data(), outs[1].GetTensorData<float>(), state_.size() * sizeof (float));

        return outs[0].GetTensorData<float>()[0];
    }

    void reset()
    {
        std::fill (state_.begin(), state_.end(), 0.0f);
    }

private:
    Ort::Env           env_;
    Ort::Session       session_;
    std::vector<float> state_;
};
