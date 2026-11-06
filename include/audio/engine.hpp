#pragma once

#include "containers.hpp"
#include <atomic>
#include <miniaudio.h>
#include <portaudio.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define RING_BUFFER_SIZE 8192
#define FRAME_REPORT_SIZE 128
#define FRAMES_PER_BLOCK 256
#define SAMPLE_RATE 48000

namespace audio {

class AudioStreamWorker;

struct AudioPlaybackReport {
  double rms;
  double estimatedPlaybackAt;
};
struct AudioPlaybackThreadData {
  RingBuffer<float, RING_BUFFER_SIZE> *bufferPtr;
  RingBuffer<AudioPlaybackReport, FRAME_REPORT_SIZE> *frameReportBufferPtr;
  const std::atomic<double> *durationPtr;
};
struct AudioFileStreamInfo {
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
enum class AudioFileStreamStatus {
  NoStream,
  Initializing,
  Ready,
  Done,
  Error,
};

enum class AudioFileStreamError {
  NoError,
  ErrorNoParentEngine,
  ErrorCouldNotLoadFile,
  ErrorCouldNotStartResampler,
  ErrorNoActiveAudioDevice,
  ErrorSampleRead,
};

typedef RingBuffer<AudioPlaybackReport, FRAME_REPORT_SIZE>
    AudioPlaybackReportRingBuffer;
typedef RingBuffer<float, RING_BUFFER_SIZE> AudioFileStreamRingBuffer;

/**
 * @brief Manages initialization and teardown of audio libraries, audio state
 * and handles audio playback, file streaming, and reports statistics of
 * streamed audio.
 *
 * The AudioEngine class manages the initialization of the portaudio library.
 * On initialization it will initialize the library, query devices, and select a
 * default device dependent on what the host API reports. The class will attempt
 * to open a stream on the default output device on initialization. This stream
 * will be closed and another will be opened when a new output device is
 * selected.
 */
class AudioEngine {
public:
  /**
   * Performs the portaudio initialization and initial device querying.
   */
  AudioEngine(const std::atomic<double> *currentTimePtr);
  /**
   * Closes any open stream and terminates portaudio.
   */
  ~AudioEngine();
  AudioEngine(const AudioEngine &engine) = delete;
  AudioEngine &operator=(const AudioEngine &engine) = delete;

  /**
   * @brief Returns the list of AudioDevices gathered during the class
   * initialization.
   */
  const std::vector<AudioDevice> &GetAudioDevices() const {
    return audioDevices_;
  }

  /**
   * @brief Returns the currently selected input device.
   */
  const AudioDevice &GetCurrentInputDevice() const {
    return currentInputDevice_;
  }
  /**
   * @brief Returns the currently selected output device.
   */
  const AudioDevice &GetCurrentOutputDevice() const {
    return currentOutputDevice_;
  }

  /**
   * @brief Returns a vector of all AudioDevices where `device.inputChannels >
   * 0`.
   */
  std::vector<AudioDevice> GetInputDevices() const;
  /**
   * @brief Returns a vector of all AudioDevices where `device.outputChannels>
   * 0`.
   */
  std::vector<AudioDevice> GetOutputDevices() const;

  /**
   * @brief Updates the current output device identified by the host deviceId
   * provided by portaudio.
   *
   * @param [in] deviceId Host API deviceId provided via an AudioDevice.id
   */
  bool SetOutputDevice(int deviceId);

  /**
   * @brief Loads an audio file via a system path and creates a file streaming
   * thread. Only one file streaming thread may be open at a time, and a
   * previously running thread will be destroyed before loading the new file.
   *
   * @param [in] sourceAudioPath The system path to an audio file. Can be .wav,
   * .flac, .mp3
   */
  void LoadSourceAudioFile(const std::string &sourceAudioPath);

  /**
   * @brief Fetches the current state of the AudioFileStream.
   *
   * @retval AudioFileStreamStatus::NoStream if there is no open file stream
   * @retval AudioFileStreamStatus::Error if there is an error in the file
   * stream
   */
  AudioFileStreamStatus GetAudioFileStreamStatus() const;
  /**
   * @brief Fetches the error reported by the current AudioFileStream.
   *
   * @retval AudioFileStreamError::NoError if there is no error.
   */
  AudioFileStreamError GetAudioFileStreamError() const;
  /**
   * @brief Returns information about the audio file currently loaded in the
   * AudioFileStream. NOTE: Calling this function when GetAudioFileStreamStatus
   * is Initializing or Error may cause undefined behavior.
   */
  AudioFileStreamInfo GetAudioFileStreamInfo() const;

  /**
   * @brief Returns a buffer of AudioPlaybackReports produced by the audio
   * playback thread. This is only intended to be consumed by the thread who
   * owns the AudioEngine. Producing to this buffer may cause undefined
   * behavior.
   *
   * @return Reference to ring buffer of AudioPlaybackReports.
   */
  AudioPlaybackReportRingBuffer &GetAudioPlaybackReportBuffer() {
    return frameReportBuffer_;
  }

  /**
   * @brief Fetches the version reported by the portaudio library.
   */
  int GetPortAudioVersion();

private:
  /**
   * @brief Helper function that closes any existing portaudio stream, and
   * attempts to start a new stream on a provided AudioDevice.
   *
   * If the function fails, it will store nullptr in audioStreamPtr_
   *
   * @param audioDevice The AudioDevice where the portaudio stream should be
   * opened.
   */
  void startAudioStream_(AudioDevice audioDevice);

  bool engineInitialized_ = false;
  std::vector<AudioDevice> audioDevices_;

  PaStream *audioStreamPtr_ = nullptr;
  const std::atomic<double> *currentTimePtr_;

  std::unique_ptr<AudioStreamWorker> audioFileStreamWorker_ = nullptr;

  AudioFileStreamRingBuffer streamBuffer_;
  AudioPlaybackReportRingBuffer frameReportBuffer_;

  AudioPlaybackThreadData audioThreadData_; // This is data that is given to the
                                            // AudioPlayback thread

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

    streamStatus_.store(AudioFileStreamStatus::Initializing);
    thread_ = std::thread(&AudioStreamWorker::run_, this);
  }
  AudioStreamWorker(const AudioStreamWorker &worker) = delete;
  AudioStreamWorker &operator=(const AudioStreamWorker &worker) = delete;
  AudioStreamWorker &operator=(AudioStreamWorker &&worker) = delete;

  AudioFileStreamStatus Status() const { return streamStatus_.load(); }
  AudioFileStreamError ErrorType() const { return errorType_.load(); }

  long GetFramesRead() const { return frameOffset_.load(); }
  double GetCurrentTimeMs() const {
    return ((double)frameOffset_.load() / streamInfo_.sampleRate) * 1000;
  }

  AudioFileStreamInfo GetStreamInfo() const {
    assert(streamStatus_.load() != AudioFileStreamStatus::Initializing);

    return streamInfo_;
  }

private:
  void run_();

  std::atomic<long> frameOffset_{0};
  AudioFileStreamInfo streamInfo_;

  std::string sourceAudioPath_;
  AudioFileStreamInfo sourceAudioInfo_;

  std::atomic<bool> shouldStop_{false};
  std::atomic<AudioFileStreamStatus> streamStatus_{
      AudioFileStreamStatus::Initializing};
  std::atomic<AudioFileStreamError> errorType_{AudioFileStreamError::NoError};

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
