#include "audio/algorithms.hpp"
#include <cassert>
#include <cmath>

void audio::FastFourierTransform(float *in, std::complex<float> *out,
                                 std::size_t count, int stride) {
  assert(in != nullptr);
  assert(out != nullptr);
  assert(count > 0 && (count & (count - 1)) == 0);
  assert(stride > 0);

  if (count == 1) {
    out[0] = in[0];
  } else {
    FastFourierTransform(in, out, count / 2, 2 * stride);
    FastFourierTransform(in + stride, out + (count / 2), count / 2, 2 * stride);

    for (int i = 0; i < (count / 2); i++) {
      std::complex<float> p = out[i];
      std::complex<float> r =
          std::polar(1.0f, (float)(-2.0f * M_PI * i / count));
      std::complex<float> q = r * out[i + (count / 2)];
      out[i] = p + q;
      out[i + (count / 2)] = p - q;
    }
  }
}

// if needed to optimize, could precompute the sine functions
void audio::ApplyHannWindow(float *in, std::size_t count) {
  for (int i = 0; i < count; i++) {
    in[i] *= std::sin(M_PI * i / count) * std::sin(M_PI * i / count);
  }
}
