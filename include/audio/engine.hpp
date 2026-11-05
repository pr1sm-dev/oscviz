#pragma once

#include "containers.hpp"
#include <atomic>
#include <miniaudio.h>
#include <portaudio.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define RING_BUFFER_SIZE 8192
#define SAMPLE_RATE 48000

namespace audio {

class AudioStreamWorker;

struct AudioStreamInfo {
  unsigned long sizeInFrames;
  double lengthMs;
  unsigned long sampleRate;
  unsigned int channels;
};
struct AudioDevice {
  int id;
  bool isDefault;
  bool isInput;
  bool isOutput;
  int inputChannels;
  int outputChannels;
  double lowInputLatency;
  double highInputLatency;
  double lowOutputLatency;
  double highOutputLatency;
  double defaultSampleRate;
  std::string name;
};
enum class AudioStreamStatus {
  NoStream,
  Initializing,
  Ready,
  Done,
  Error,
};

enum class AudioStreamError {
  NoError,
  ErrorNoParentEngine,
  ErrorCouldNotLoadFile,
  ErrorCouldNotStartResampler,
  ErrorNoActiveAudioDevice,
  ErrorSampleRead,
};

class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine();
  AudioEngine(const AudioEngine &engine) = delete;
  AudioEngine &operator=(const AudioEngine &engine) = delete;

  bool Initialize();
  const std::vector<AudioDevice> &GetAudioDevices() const {
    return audioDevices_;
  }

  const AudioDevice &GetCurrentInputDevice() const {
    return currentInputDevice_;
  }
  const AudioDevice &GetCurrentOutputDevice() const {
    return currentOutputDevice_;
  }

  std::vector<AudioDevice> GetInputDevices() const;
  std::vector<AudioDevice> GetOutputDevices() const;

  bool SetOutputDevice(int deviceId);

  void LoadSourceAudioFile(const std::string &sourceAudioPath);
  AudioStreamStatus GetAudioStreamStatus() const;
  AudioStreamError GetAudioStreamError() const;
  unsigned long GetAudioStreamSizeInFrames() const;
  double GetAudioStreamLengthMs() const;
  AudioStreamInfo GetAudioStreamInfo() const;

  int GetPortAudioVersion();

private:
  void startAudioStream_(AudioDevice audioDevice);

  std::vector<AudioDevice> audioDevices_;
  bool initialized_ = false;

  PaStream *audioStream_ = nullptr;

  std::unique_ptr<AudioStreamWorker> streamWorker_ = nullptr;

  RingBuffer<float, RING_BUFFER_SIZE> streamBuffer_;

  AudioDevice currentInputDevice_;
  AudioDevice currentOutputDevice_;
};

class AudioStreamWorker {
public:
  AudioStreamWorker(AudioEngine *engine, const std::string &sourceAudioPath,
                    RingBuffer<float, RING_BUFFER_SIZE> &buffer)
      : sourceAudioPath_(sourceAudioPath), parentEngine_(engine),
        ringBuffer_(buffer), thread_(&AudioStreamWorker::run_, this) {}

  ~AudioStreamWorker() {
    shouldStop_.store(true);
    if (thread_.joinable())
      thread_.join();
  }
  AudioStreamWorker(AudioStreamWorker &&worker) noexcept
      : sourceAudioPath_(std::move(worker.sourceAudioPath_)),
        parentEngine_(std::move(worker.parentEngine_)),
        ringBuffer_(worker.ringBuffer_) {
    worker.shouldStop_.store(true);
    if (worker.thread_.joinable()) {
      worker.thread_.join();
    }

    streamStatus_.store(AudioStreamStatus::Initializing);
    thread_ = std::thread(&AudioStreamWorker::run_, this);
  }
  AudioStreamWorker(const AudioStreamWorker &worker) = delete;
  AudioStreamWorker &operator=(const AudioStreamWorker &worker) = delete;
  AudioStreamWorker &operator=(AudioStreamWorker &&worker) = delete;

  AudioStreamStatus Status() const { return streamStatus_.load(); }
  AudioStreamError ErrorType() const { return errorType_.load(); }

  long GetFramesRead() const { return frameOffset_.load(); }
  double GetCurrentTimeMs() const {
    return ((double)frameOffset_.load() / streamInfo_.sampleRate) * 1000;
  }

  AudioStreamInfo GetStreamInfo() const {
    assert(streamStatus_.load() != AudioStreamStatus::Initializing);

    return streamInfo_;
  }

private:
  void run_();

  std::atomic<long> frameOffset_{0};
  AudioStreamInfo streamInfo_;

  std::string sourceAudioPath_;
  AudioStreamInfo sourceAudioInfo_;

  std::atomic<bool> shouldStop_{false};
  std::atomic<AudioStreamStatus> streamStatus_{AudioStreamStatus::Initializing};
  std::atomic<AudioStreamError> errorType_{AudioStreamError::NoError};

  AudioEngine *parentEngine_ = nullptr;
  RingBuffer<float, RING_BUFFER_SIZE> &ringBuffer_;

  std::thread thread_;
};

std::vector<AudioDevice> getAudioDevices_();

int audioDeviceStreamCallback_(const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData);
} // namespace audio

// namespace audio
