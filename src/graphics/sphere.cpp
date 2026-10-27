#include "graphics/sphere.hpp"
#include <cmath>

Sphere::Sphere(float radius, int nOfLatitudes, int nOfLongitudes)
    : radius_(radius), nOfLongitudes_(nOfLongitudes),
      nOfLatitudes_(nOfLatitudes) {
  if (radius <= 0 || nOfLatitudes <= 0 || nOfLongitudes <= 0)
    return;

  int nOfVertices = (nOfLongitudes * nOfLatitudes) + 2;

  // bottom point
  vertices_.push_back(0.0f);
  vertices_.push_back(0.0f);
  vertices_.push_back(-radius);

  for (int lat = 1; lat <= nOfLatitudes; lat++) {
    double phi = (M_PI)*lat / (nOfLatitudes + 1);
    for (int lon = 0; lon < nOfLongitudes; lon++) {
      double theta = (2 * M_PI) * lon / nOfLongitudes;

      double x = radius * sin(phi) * cos(theta);
      double y = radius * sin(phi) * sin(theta);
      double z = -radius * cos(phi);

      vertices_.push_back(x);
      vertices_.push_back(y);
      vertices_.push_back(z);
    }
  }

  // top point
  vertices_.push_back(0.0f);
  vertices_.push_back(0.0f);
  vertices_.push_back(radius);

  // bottom fan, all connect to bottom
  for (int i = 0; i < nOfLongitudes - 1; i++) {
    indices_.push_back(0);
    indices_.push_back(2 + i);
    indices_.push_back(1 + i);
  }
  indices_.push_back(0);
  indices_.push_back(nOfLongitudes);
  indices_.push_back(1);

  for (int lat = 0; lat < nOfLatitudes - 1; lat++) {
    for (int lon = 0; lon < nOfLongitudes - 1; lon++) {
      indices_.push_back(1 + (lat * nOfLongitudes) + lon);       // bot-left
      indices_.push_back(1 + ((lat + 1) * nOfLongitudes) + lon); // top-left
      indices_.push_back(2 + (lat * nOfLongitudes) + lon);       // bot- right
      indices_.push_back(1 + ((lat + 1) * nOfLongitudes) + lon); // top-left
      indices_.push_back(2 + ((lat + 1) * nOfLongitudes) + lon); // top-right-
      indices_.push_back(2 + (lat * nOfLongitudes) + lon);       // bot- right
    }
    indices_.push_back(1 + (lat * nOfLongitudes) +
                       (nOfLongitudes - 1)); // bot-left
    indices_.push_back(1 + ((lat + 1) * nOfLongitudes) +
                       (nOfLongitudes - 1));       // top-left
    indices_.push_back(1 + (lat * nOfLongitudes)); // bot- right
    indices_.push_back(1 + ((lat + 1) * nOfLongitudes) +
                       (nOfLongitudes - 1));             // top-left
    indices_.push_back(1 + ((lat + 1) * nOfLongitudes)); // top-right-
    indices_.push_back(1 + (lat * nOfLongitudes));       // bot- right
  }

  // top fan, all connect to top
  // first vertex will be located at
  for (int i = 0; i < nOfLongitudes - 1; i++) {
    indices_.push_back(1 + ((nOfLatitudes - 1) * nOfLongitudes) + i);
    indices_.push_back(2 + ((nOfLatitudes - 1) * nOfLongitudes) + i);
    indices_.push_back(nOfVertices - 1);
  }
  indices_.push_back(nOfLatitudes * nOfLongitudes);
  indices_.push_back(1 + ((nOfLatitudes - 1) * nOfLongitudes));
  indices_.push_back(nOfVertices - 1);
}
