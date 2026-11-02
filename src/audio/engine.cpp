#include "audio/engine.hpp"
#include <algorithm>
#include <iostream>
#include <portaudio.h>
#include <stdexcept>

namespace audio {
AudioEngine::AudioEngine() {
  PaError error = Pa_Initialize();
  if (error != paNoError) {
    std::cout << "ERROR: Could not initialize PortAudio, error code 0x"
              << std::hex << error << std::endl;
    throw std::invalid_argument("ERROR: Could not initialize PortAudio");
    return;
  }

  audioDevices_ = getAudioDevices_();
  for (const AudioDevice &d : audioDevices_) {
    if (d.isDefault && d.isInput) {
      currentInputDevice_ = d;
    } else if (d.isDefault && d.isOutput) {
      currentOutputDevice_ = d;
    }
  }
}

AudioEngine::~AudioEngine() { Pa_Terminate(); }

std::vector<AudioDevice> AudioEngine::GetInputDevices() const {
  std::vector<AudioDevice> inputDevices;

  std::copy_if(audioDevices_.begin(), audioDevices_.end(),
               std::back_inserter(inputDevices),
               [](const AudioDevice &d) { return d.isInput; });

  return inputDevices;
}

bool AudioEngine::SetOutputDevice(int deviceId) {
  auto newOutputDevice = std::find_if(
      audioDevices_.begin(), audioDevices_.end(),
      [deviceId](const AudioDevice &device) { return device.id == deviceId; });

  if (newOutputDevice == audioDevices_.end())
    return false;

  currentOutputDevice_ = *newOutputDevice;

  return true;
}

std::vector<AudioDevice> AudioEngine::GetOutputDevices() const {
  std::vector<AudioDevice> outputDevices;

  std::copy_if(audioDevices_.begin(), audioDevices_.end(),
               std::back_inserter(outputDevices),
               [](const AudioDevice &d) { return d.isOutput; });

  return outputDevices;
}

std::vector<AudioDevice> getAudioDevices_() {
  std::vector<AudioDevice> devices;

  int defaultInputDevice = Pa_GetDefaultInputDevice();
  int defaultOutputDevice = Pa_GetDefaultOutputDevice();

  int numberOfDevices = Pa_GetDeviceCount();
  for (int i = 0; i < numberOfDevices; i++) {

    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
    if (deviceInfo) {
      AudioDevice audioDevice;

      audioDevice.id = i;
      audioDevice.isDefault =
          i == defaultInputDevice || i == defaultOutputDevice;
      audioDevice.inputChannels = deviceInfo->maxInputChannels;
      audioDevice.outputChannels = deviceInfo->maxOutputChannels;
      audioDevice.highInputLatency = deviceInfo->defaultHighInputLatency;
      audioDevice.lowInputLatency = deviceInfo->defaultLowInputLatency;
      audioDevice.highOutputLatency = deviceInfo->defaultHighOutputLatency;
      audioDevice.lowOutputLatency = deviceInfo->defaultLowOutputLatency;
      audioDevice.name = deviceInfo->name;

      audioDevice.isInput = audioDevice.inputChannels > 0;
      audioDevice.isOutput = audioDevice.outputChannels > 0;

      devices.push_back(audioDevice);
    }
  }

  return devices;
}

int AudioEngine::GetPortAudioVersion() { return Pa_GetVersion(); }

} // namespace audio
