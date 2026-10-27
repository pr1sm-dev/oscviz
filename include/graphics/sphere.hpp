#pragma once

#include <vector>
class Sphere {
public:
  Sphere(float radius, int nOflatitudes, int nOfLongitudes);

  const std::vector<float> &GetVertices() const { return vertices_; }
  const std::vector<int> &GetIndices() const { return indices_; }

private:
  int nOfLatitudes_;
  int nOfLongitudes_;
  float radius_;

  std::vector<float> vertices_;
  std::vector<int> indices_;
};
