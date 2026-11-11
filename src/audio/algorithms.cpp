#include "audio/algorithms.hpp"
#include <cassert>
#include <cmath>

void audio::FastFourierTransform(double *in, std::complex<double> *out,
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
      std::complex<double> p = out[i];
      std::complex<double> q =
          std::polar(1.0, -2.0 * M_PI * i / count) * out[i + (count / 2)];
      out[i] = p + q;
      out[i + (count / 2)] = p - q;
    }
  }
}
