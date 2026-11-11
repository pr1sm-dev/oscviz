#include "audio/algorithms.hpp"
#include <complex>
#include <gtest/gtest.h>
#include <random>

// TODO: test the initial parameters

// TODO: Test at all input points
TEST(FastFourierTransform, DeltaFunction) {
  std::vector<double> x(512);
  std::vector<std::complex<double>> X(x.size());

  x[0] = 1;

  audio::FastFourierTransform(x.data(), X.data(), x.size());

  for (auto i : X) {
    EXPECT_EQ(i, std::complex<double>(1.0, 0.0))
        << "FFT did not return 1 when the input was the Kronecker "
           "delta function";
  }
}

TEST(FastFourierTransform, RandomLinearity) {
  std::mt19937 rng(1700923);
  std::uniform_real_distribution<double> dist(-1.0, 1.0);

  std::vector<int> sizes{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

  for (int n = 0; n < sizes.size(); n++) {
    std::vector<double> x1(sizes[n]);
    std::vector<double> x2(x1.size());
    std::vector<double> x3(x1.size());
    std::vector<std::complex<double>> X1(x1.size());
    std::vector<std::complex<double>> X2(x2.size());
    std::vector<std::complex<double>> X3(x3.size());

    double a = dist(rng);
    double b = dist(rng);

    for (int i = 0; i < x1.size(); i++) {
      x1[i] = dist(rng);
      x2[i] = dist(rng);
      x3[i] = a * x1[i] + b * x2[i];
    }

    audio::FastFourierTransform(x1.data(), X1.data(), x1.size());
    audio::FastFourierTransform(x2.data(), X2.data(), x2.size());
    audio::FastFourierTransform(x3.data(), X3.data(), x3.size());

    for (int i = 0; i < x1.size(); i++) {
      std::complex<double> res = a * X1[i] + b * X2[i];
      EXPECT_LT(std::abs((res - X3[i])), 1e-9)
          << "linearity did not hold for the FFT function";
    }
  }
}
