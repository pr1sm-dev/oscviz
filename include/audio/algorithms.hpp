#include <complex>
#include <cstddef>

namespace audio {

/**
 * @brief Computes the Discrete Fourier Transform for a list of double samples
 * using the Fast Fourier Transform algorithm.
 *
 * @param in The input samples given as a list of doubles.
 * @param count The number of samples. This number defines the length of both in
 * and out lists.
 * @param out The output samples given as a list of std::complex<double>
 * @param stride Defaults to 1, used for the recursive algorithm
 */
void FastFourierTransform(float *in, std::complex<float> *out,
                          std::size_t count, int stride = 1);

/**
 * @brief Applies a Hann window to a discrete set of n samples.
 *
 * @param in The input samples given as a list of doubles.
 * @param count The number of samples.
 */
void ApplyHannWindow(float *in, std::size_t count);
} // namespace audio
