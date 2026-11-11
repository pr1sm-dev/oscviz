#include "audio/engine.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <portaudio.h>
#include <stdexcept>
#include <thread>

namespace audio {
AudioEngine::AudioEngine(const std::atomic<double> *currentTimePtr)
    : currentTimePtr_(currentTimePtr) {
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

  // TODO: add proper check here, should be if there even is a current input
  // device
  startAudioStream_(currentOutputDevice_);
}

AudioEngine::~AudioEngine() {
  if (audioStreamPtr_) {
    Pa_CloseStream(audioStreamPtr_);
  }
  Pa_Terminate();
}

std::vector<AudioDevice> AudioEngine::GetInputDevices() const {
  std::vector<AudioDevice> inputDevices;

  std::copy_if(audioDevices_.begin(), audioDevices_.end(),
               std::back_inserter(inputDevices),
               [](const AudioDevice &d) { return d.isInput; });

  return inputDevices;
}

void AudioEngine::startAudioStream_(AudioDevice audioDevice) {
  if (audioStreamPtr_ != nullptr) {
    Pa_CloseStream(audioStreamPtr_);
    audioStreamPtr_ = nullptr;
  }
  int channelCount =
      audioDevice.outputChannels < 2 ? audioDevice.outputChannels : 2;

  PaStreamParameters deviceParameters = {.device = audioDevice.id,
                                         .channelCount = channelCount,
                                         .sampleFormat = paFloat32,
                                         .suggestedLatency = 0.01,
                                         .hostApiSpecificStreamInfo = NULL};

  audioThreadData_ = {.bufferPtr = &streamBuffer_,
                      .frameReportBufferPtr = &frameReportBuffer_,
                      .durationPtr = currentTimePtr_};

  PaError streamInitializationError = Pa_OpenStream(
      &audioStreamPtr_, NULL, &deviceParameters, SAMPLE_RATE, FRAMES_PER_BLOCK,
      paNoFlag, &audioDeviceStreamCallback_, &audioThreadData_);
  if (streamInitializationError != paNoError) {
    std::cout << "ERROR: Could not open default stream, error code 0x"
              << std::hex << streamInitializationError << std::endl;
  } else {
    Pa_StartStream(audioStreamPtr_);
  }
}

bool AudioEngine::SetOutputDevice(int deviceId) {
  auto newOutputDevice = std::find_if(
      audioDevices_.begin(), audioDevices_.end(),
      [deviceId](const AudioDevice &device) { return device.id == deviceId; });

  if (newOutputDevice == audioDevices_.end())
    return false;

  currentOutputDevice_ = *newOutputDevice;

  if (audioStreamPtr_) {
    std::cout << "closing stream..." << std::endl;
    Pa_CloseStream(audioStreamPtr_);
    audioStreamPtr_ = nullptr;
  }

  // TODO: Break this out into its own function and add error handling so the
  // main thread knows if it worked
  startAudioStream_(currentOutputDevice_);

  std::cout << "current stream: " << audioStreamPtr_ << std::endl;

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
  if (audioFileStreamWorker_) {
    audioFileStreamWorker_.reset(nullptr);
  }

  audioFileStreamWorker_ =
      std::make_unique<AudioStreamWorker>(this, sourceAudioPath, streamBuffer_);
}

AudioFileStreamStatus AudioEngine::GetAudioFileStreamStatus() const {
  if (!audioFileStreamWorker_)
    return AudioFileStreamStatus::NoStream;

  return audioFileStreamWorker_->Status();
}

AudioFileStreamError AudioEngine::GetAudioFileStreamError() const {
  if (!audioFileStreamWorker_)
    return AudioFileStreamError::NoError;

  return audioFileStreamWorker_->ErrorType();
}

AudioFileStreamInfo AudioEngine::GetAudioFileStreamInfo() const {
  assert(audioFileStreamWorker_);

  return audioFileStreamWorker_->GetStreamInfo();
}

AudioStreamWorker::~AudioStreamWorker() {
  shouldStop_.store(true);
  if (thread_.joinable())
    thread_.join();

  if (decoderInitialized_) {
    ma_decoder_uninit(&decoder_);
  }
  if (converterInitialized_) {
    ma_data_converter_uninit(&converter_, NULL);
  }
}

void AudioStreamWorker::initialize_() {
  if (!parentEnginePtr_) {
    errorType_.store(AudioFileStreamError::ErrorNoParentEngine);
    streamStatus_.store(AudioFileStreamStatus::Error);
    return;
  }

  // Note: if there is an issue with no active device, this is probably where...
  const AudioDevice &outputDevice = parentEnginePtr_->GetCurrentOutputDevice();

  // TODO: check valid device?

  // Open the file stream
  ma_decoder_config decoderConfig = ma_decoder_config_init(
      ma_format_f32,
      outputDevice.outputChannels > 2 ? 2 : outputDevice.outputChannels, 0);
  ma_result initializeDecoderResult =
      ma_decoder_init_file(sourceAudioPath_.c_str(), &decoderConfig, &decoder_);
  if (initializeDecoderResult != MA_SUCCESS) {
    errorType_.store(AudioFileStreamError::ErrorCouldNotLoadFile);
    streamStatus_.store(AudioFileStreamStatus::Error);
    return;
  }
  decoderInitialized_ = true;

  // start the resampler
  ma_data_converter_config dataConverterConfig = ma_data_converter_config_init(
      decoder_.outputFormat, ma_format_f32, decoder_.outputChannels,
      decoder_.outputChannels, decoder_.outputSampleRate, SAMPLE_RATE);
  ma_result inititalizeConverterResult =
      ma_data_converter_init(&dataConverterConfig, NULL, &converter_);

  if (inititalizeConverterResult != MA_SUCCESS) {
    errorType_.store(AudioFileStreamError::ErrorCouldNotStartResampler);
    streamStatus_.store(AudioFileStreamStatus::Error);
    return;
  }
  converterInitialized_ = true;

  ma_uint64 nofFrames;
  ma_result getFileSizeResult =
      ma_data_source_get_length_in_pcm_frames(&decoder_, &nofFrames);
  if (getFileSizeResult != MA_SUCCESS) {
    errorType_.store(AudioFileStreamError::ErrorCouldNotLoadFile);
    streamStatus_.store(AudioFileStreamStatus::Error);
    return;
  }

  // Store metadata about the file
  streamInfo_.sizeInFrames = nofFrames;
  streamInfo_.sampleRate = decoder_.outputSampleRate;
  streamInfo_.channels = decoder_.outputChannels;
  streamInfo_.lengthMs =
      ((double)streamInfo_.sizeInFrames / streamInfo_.sampleRate) * 1000;
}

void AudioStreamWorker::run_() {
  sleep(1);
  initialize_();
  if (streamStatus_.load() == AudioFileStreamStatus::Error)
    return;

  // Now we begin
  streamStatus_.store(AudioFileStreamStatus::Ready);

  float audioFrames[RING_BUFFER_SIZE]; // can store (RING_BUFFER_SIZE /
                                       // channels) number of frames
  float audioFramesResampled[RING_BUFFER_SIZE];
  while (!shouldStop_.load() &&
         (streamStatus_.load() == AudioFileStreamStatus::Ready)) {
    if (streamStatus_.load() == AudioFileStreamStatus::Paused) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      continue;
    }

    int framesAvailable = ringBuffer_.remaining() / decoder_.outputChannels;

    // if the audio thread is consuming too quickly, wait a bit to not burn the
    // CPU
    if (framesAvailable == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    // Get the number of frames to read, so that when resampled it can fill the
    // ring buffer
    ma_uint64 inputFramesRequired;
    ma_data_converter_get_required_input_frame_count(
        &converter_, framesAvailable, &inputFramesRequired);

    ma_uint64 framesRead;
    ma_result readResult = ma_decoder_read_pcm_frames(
        &decoder_, audioFrames, inputFramesRequired, &framesRead);
    if (readResult != MA_SUCCESS && readResult != MA_AT_END) {
      errorType_.store(AudioFileStreamError::ErrorSampleRead);
      streamStatus_.store(AudioFileStreamStatus::Error);
      break;
    }

    ma_uint64 framesIn = framesRead;
    ma_uint64 framesOut = framesAvailable;

    ma_result resampleResult = ma_data_converter_process_pcm_frames(
        &converter_, audioFrames, &framesIn, audioFramesResampled, &framesOut);
    if (resampleResult != MA_SUCCESS) {
      errorType_.store(AudioFileStreamError::ErrorSampleRead);
      streamStatus_.store(AudioFileStreamStatus::Error);
      break;
    }

    ringBuffer_.write(audioFramesResampled,
                      framesOut * decoder_.outputChannels);

    if (readResult == MA_AT_END) {
      streamStatus_.store(AudioFileStreamStatus::Paused);
    }

    // This is to not burn the CPU, improvements might make this dynamic
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int audioDeviceStreamCallback_(const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData) {
  // FOR NOW ASSUME 2 CHANNELS
  AudioPlaybackThreadData *threadDataPtr = (AudioPlaybackThreadData *)userData;
  float *outFrames = (float *)outputBuffer;

  unsigned int dataAvailable = threadDataPtr->bufferPtr->size();
  unsigned int framesToWrite = (dataAvailable / 2) < framesPerBuffer
                                   ? (dataAvailable / 2)
                                   : framesPerBuffer;

  double lRms = 0.0;
  double rRms = 0.0;

  if (framesToWrite > 0) {
    threadDataPtr->bufferPtr->read(outFrames, framesToWrite * 2);

    for (int i = 0; i < framesToWrite; i++) {
      lRms += outFrames[2 * i] * outFrames[2 * i];
      rRms += outFrames[2 * i + 1] * outFrames[2 * i + 1];
    }
    lRms = std::sqrt(lRms / framesToWrite);
    rRms = std::sqrt(rRms / framesToWrite);
  }

  if (!threadDataPtr->frameReportBufferPtr->is_full()) {
    double expectedPlayAt =
        threadDataPtr->durationPtr->load() +
        (timeInfo->outputBufferDacTime - timeInfo->currentTime);

    AudioPlaybackReport frameReport = {
        .lRms = lRms, .rRms = rRms, .estimatedPlaybackAt = expectedPlayAt};
    threadDataPtr->frameReportBufferPtr->push(frameReport);
  }

  // fill the rest with silence
  if (framesToWrite < framesPerBuffer) {
    for (int i = 0; i < framesPerBuffer - framesToWrite; i++) {
      outFrames[framesToWrite * 2 + 2 * i] = 0;
      outFrames[framesToWrite * 2 + 2 * i + 1] = 0;
    }
  }

  return 0;
}

} // namespace audio
