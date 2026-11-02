#pragma once

#include <vector>

namespace audio {

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

  int GetPortAudioVersion();

private:
  std::vector<AudioDevice> audioDevices_;
  bool initialized_ = false;

  AudioDevice currentInputDevice_;
  AudioDevice currentOutputDevice_;
};

std::vector<AudioDevice> getAudioDevices_();
} // namespace audio
