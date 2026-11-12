#include "audio/algorithms.hpp"
#include <cmath>
#include <complex>
#include <gtest/gtest.h>
#include <random>

// TODO: test the initial parameters

TEST(FastFourierTransform, DeltaFunction) {
  std::vector<int> sizes{
      1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
  };

  for (auto size : sizes) {
    for (int i = 0; i < size; i++) {
      std::vector<float> x(size);
      std::vector<std::complex<float>> X(x.size());
      x[i] = 1;
      audio::FastFourierTransform(x.data(), X.data(), x.size());

      for (int k = 0; k < x.size(); k++) {
        std::complex<float> res =
            std::polar(1.0f, (float)(-2 * M_PI * k * i / x.size()));
        EXPECT_LT(std::abs(X[k] - res), 1e-3)
            << "FFT did not return 1 when the input was the Kronecker "
               "delta function";
      }
    }
  }
}

TEST(FastFourierTransform, RandomLinearity) {
  std::mt19937 rng(1700923);
  std::uniform_real_distribution<float> dist(-1.0, 1.0);

  std::vector<int> sizes{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

  for (int n = 0; n < sizes.size(); n++) {
    std::vector<float> x1(sizes[n]);
    std::vector<float> x2(x1.size());
    std::vector<float> x3(x1.size());
    std::vector<std::complex<float>> X1(x1.size());
    std::vector<std::complex<float>> X2(x2.size());
    std::vector<std::complex<float>> X3(x3.size());

    float a = dist(rng);
    float b = dist(rng);

    for (int i = 0; i < x1.size(); i++) {
      x1[i] = dist(rng);
      x2[i] = dist(rng);
      x3[i] = a * x1[i] + b * x2[i];
    }

    audio::FastFourierTransform(x1.data(), X1.data(), x1.size());
    audio::FastFourierTransform(x2.data(), X2.data(), x2.size());
    audio::FastFourierTransform(x3.data(), X3.data(), x3.size());

    for (int i = 0; i < x1.size(); i++) {
      std::complex<float> res = a * X1[i] + b * X2[i];
      EXPECT_LT(std::abs((res - X3[i])), 1e-3)
          << "linearity did not hold for the FFT function";
    }
  }
}

/**TEST(FastFourierTransform, SinWave) {
  // 220 hz cos wave
  // consider sample rale of 880hz
  // 32 bins

  std::vector<float> samples(256);
  std::vector<std::complex<float>> freq(samples.size());
  double step = (1.0 / 880.0);
  for (int i = 0; i < samples.size(); i++) {
    samples[i] = 0.32 * std::cos(2 * M_PI * 220 * step * i) +
                 std::cos(2 * M_PI * 100 * step * i);
  }

  float min = samples[0], max = samples[0];
  for (int i = 1; i < samples.size(); i++) {
    if (samples[i] < min)
      min = samples[i];
    if (samples[i] > max)
      max = samples[i];
  }
  float center = (min + max) / 2.0;
  float size = (max - min) / 2.0;
  for (int i = 0; i < samples.size(); i++) {
    samples[i] = (1.0 / size) * (samples[i] + (0.0 - center));
  }

  audio::ApplyHannWindow(samples.data(), samples.size());

  audio::FastFourierTransform(samples.data(), freq.data(), samples.size());

  for (int i = 0; i < samples.size(); i++) {
    freq[i] = freq[i] * (4.0f / samples.size());
  }

  for (int i = 0; i < samples.size(); i++) {
    std::cout << freq[i] << " - " << std::abs(freq[i]) << " | "
              << 20 * std::log10(std::abs(freq[i])) << " - "
              << (880.0 * i) / samples.size() << std::endl;
  }
}*/
