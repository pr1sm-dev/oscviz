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

  PaError streamInitializationError = Pa_OpenDefaultStream(
      &audioStream_, 0, currentOutputDevice_.outputChannels, paFloat32,
      SAMPLE_RATE, 256, &audioDeviceStreamCallback_, &streamBuffer_);
  if (streamInitializationError != paNoError) {
    std::cout << "ERROR: Could not open default stream, error code 0x"
              << std::hex << streamInitializationError << std::endl;
    throw std::invalid_argument("ERROR: Could not open default stream");
  }

  Pa_StartStream(audioStream_);
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

void AudioEngine::LoadSourceAudioFile(const std::string &sourceAudioPath) {
  // kill the current stream
  if (streamWorker_) {
    streamWorker_.reset(nullptr);
  }

  streamWorker_ =
      std::make_unique<AudioStreamWorker>(this, sourceAudioPath, streamBuffer_);
}

AudioStreamStatus AudioEngine::GetAudioStreamStatus() const {
  if (!streamWorker_)
    return AudioStreamStatus::NoStream;

  return streamWorker_->Status();
}

AudioStreamError AudioEngine::GetAudioStreamError() const {
  if (!streamWorker_)
    return AudioStreamError::NoError;

  return streamWorker_->ErrorType();
}

unsigned long AudioEngine::GetAudioStreamSizeInFrames() const {
  assert(streamWorker_);
  if (!streamWorker_)
    return 0;

  return streamWorker_->GetStreamInfo().sizeInFrames;
}

double AudioEngine::GetAudioStreamLengthMs() const {
  assert(streamWorker_);
  if (!streamWorker_)
    return 0;

  return streamWorker_->GetStreamInfo().lengthMs;
}

AudioStreamInfo AudioEngine::GetAudioStreamInfo() const {
  assert(streamWorker_);

  return streamWorker_->GetStreamInfo();
}

void AudioStreamWorker::run_() {
  sleep(1);
  if (!parentEngine_) {
    errorType_.store(AudioStreamError::ErrorNoParentEngine);
    streamStatus_.store(AudioStreamStatus::Error);
    return;
  }

  // Note: if there is an issue with no active device, this is probably where...
  const AudioDevice &outputDevice = parentEngine_->GetCurrentOutputDevice();

  // open the stream
  ma_decoder decoder;
  ma_data_converter converter;
  ma_decoder_config decoderConfig = ma_decoder_config_init(
      ma_format_f32,
      outputDevice.outputChannels > 2 ? 2 : outputDevice.outputChannels, 0);
  ma_result initializeDecoderResult =
      ma_decoder_init_file(sourceAudioPath_.c_str(), &decoderConfig, &decoder);
  if (initializeDecoderResult != MA_SUCCESS) {
    errorType_.store(AudioStreamError::ErrorCouldNotLoadFile);
    streamStatus_.store(AudioStreamStatus::Error);
    return;
  }

  ma_data_converter_config dataConverterConfig = ma_data_converter_config_init(
      decoder.outputFormat, ma_format_f32, decoder.outputChannels,
      decoder.outputChannels, decoder.outputSampleRate, SAMPLE_RATE);
  ma_result inititalizeConverterResult =
      ma_data_converter_init(&dataConverterConfig, NULL, &converter);

  if (inititalizeConverterResult != MA_SUCCESS) {
    errorType_.store(AudioStreamError::ErrorCouldNotStartResampler);
    streamStatus_.store(AudioStreamStatus::Error);

    ma_decoder_uninit(&decoder);
    return;
  }

  ma_uint64 nofFrames;
  ma_result getTotalSizeResult =
      ma_data_source_get_length_in_pcm_frames(&decoder, &nofFrames);
  if (getTotalSizeResult != MA_SUCCESS) {
    errorType_.store(AudioStreamError::ErrorCouldNotLoadFile);
    streamStatus_.store(AudioStreamStatus::Error);

    ma_decoder_uninit(&decoder);
    ma_data_converter_uninit(&converter, NULL);
    return;
  }

  streamInfo_.sizeInFrames = nofFrames;
  streamInfo_.sampleRate = decoder.outputSampleRate;
  streamInfo_.channels = decoder.outputChannels;
  streamInfo_.lengthMs =
      ((double)streamInfo_.sizeInFrames / streamInfo_.sampleRate) * 1000;

  streamStatus_.store(AudioStreamStatus::Ready);

  float audioFrames[RING_BUFFER_SIZE];
  float audioFramesResampled[RING_BUFFER_SIZE];
  while (!shouldStop_.load() &&
         (streamStatus_.load() == AudioStreamStatus::Ready)) {
    int framesAvailable = ringBuffer_.capacity() / decoder.outputChannels;
    // Multiply this by the resample ratio
    // we want to get x frames such that when resampled, they will at most get
    // the number of available frames
    // int framesRequired = framesAvailable * resampleRatio;

    ma_uint64 inputFramesRequired;
    ma_data_converter_get_required_input_frame_count(
        &converter, framesAvailable, &inputFramesRequired);

    if (framesAvailable == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    ma_uint64 framesRead;
    ma_result readResult = ma_decoder_read_pcm_frames(
        &decoder, audioFrames, inputFramesRequired, &framesRead);
    if (readResult != MA_SUCCESS && readResult != MA_AT_END) {
      errorType_.store(AudioStreamError::ErrorSampleRead);
      streamStatus_.store(AudioStreamStatus::Error);
      break;
    }

    ma_uint64 framesIn = framesRead;
    ma_uint64 framesOut = framesAvailable;

    ma_result resampleResult = ma_data_converter_process_pcm_frames(
        &converter, audioFrames, &framesIn, audioFramesResampled, &framesOut);
    if (resampleResult != MA_SUCCESS) {
      errorType_.store(AudioStreamError::ErrorSampleRead);
      streamStatus_.store(AudioStreamStatus::Error);
      break;
    }
    if (framesIn < framesRead)
      std::cout << "DROPPED " << (framesRead - framesIn) << " input frames\n";
    // std::cout << "Read " << framesRead << " frames." << std::endl;

    ringBuffer_.write(audioFramesResampled, framesOut * decoder.outputChannels);

    if (readResult == MA_AT_END) {
      streamStatus_.store(AudioStreamStatus::Done);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  ma_decoder_uninit(&decoder);
  ma_data_converter_uninit(&converter, NULL);

  std::cout << "ending thread!" << std::endl;
}

int audioDeviceStreamCallback_(const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData) {
  // TODO: create struct and pass in more data
  // FOR NOW ASSUME 2 CHANNELS
  RingBuffer<float, RING_BUFFER_SIZE> *buffer =
      (RingBuffer<float, RING_BUFFER_SIZE> *)userData;
  float *outFrames = (float *)outputBuffer;

  unsigned int dataAvailable = buffer->size();
  unsigned int framesToWrite = (dataAvailable / 2) < framesPerBuffer
                                   ? (dataAvailable / 2)
                                   : framesPerBuffer;

  if (framesToWrite > 0) {
    buffer->read(outFrames, framesToWrite * 2);
  }

  if (framesToWrite < framesPerBuffer) {
    std::cout << "Miss!" << std::endl;
    for (int i = 0; i < framesPerBuffer - framesToWrite; i++) {
      outFrames[framesToWrite * 2 + 2 * i] = 0;
      outFrames[framesToWrite * 2 + 2 * i + 1] = 0;
    }
  }

  return 0;
}

} // namespace audio
