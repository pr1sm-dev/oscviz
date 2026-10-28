#include "graphics/sphere.hpp"
#include <cstdlib>
#include <gtest/gtest.h>
#include <vector>

TEST(Sphere, InvalidParameters) {
  Sphere s0(0, 10, 10);

  EXPECT_EQ(s0.GetVertices().size(), 0)
      << "sphere with radius 0 had nonempty vertices";
  EXPECT_EQ(s0.GetIndices().size(), 0)
      << "sphere with radius 0 had nonempty indicies";

  Sphere s1(1.0f, 0, 10);

  EXPECT_EQ(s1.GetVertices().size(), 0)
      << "sphere with 0 latitudes had nonempty vertices";
  EXPECT_EQ(s1.GetIndices().size(), 0)
      << "sphere with 0 latitudes had nonempty indicies";

  Sphere s2(1.0f, 10, 0);

  EXPECT_EQ(s2.GetVertices().size(), 0)
      << "sphere with 0 longitudes had nonempty vertices";
  EXPECT_EQ(s2.GetIndices().size(), 0)
      << "sphere with 0 latitudes had nonempty indices";
}

TEST(Sphere, CreatesValidVerticesAndIndices) {
  int latitudes = 32;
  int longitudes = 32;
  float radius = 2.0f;
  float error = 0.0001;

  int nOfVertices = 2 + (latitudes * longitudes);
  int nOfTris = 2 * latitudes * longitudes;

  Sphere s(radius, latitudes, longitudes);
  const std::vector<float> &vertices = s.GetVertices();
  const std::vector<int> &indicies = s.GetIndices();

  ASSERT_EQ(vertices.size(), nOfVertices * 3)
      << "sphere did not generate the correct number of vertices";

  // Check each vertex to ensure it is of length radius
  for (int i = 0; i < nOfVertices; i++) {
    float x = vertices[i * 3];
    float y = vertices[i * 3 + 1];
    float z = vertices[i * 3 + 2];

    float length = x * x + y * y + z * z;
    EXPECT_LE(abs(length - (radius * radius)), error)
        << "did not hold the sphere condition";
  }

  ASSERT_EQ(s.GetIndices().size(), nOfTris * 3)
      << "sphere did not generate the correct number of indices";

  for (int i = 0; i < nOfTris; i++) {
    int x = indicies[i * 3];
    int y = indicies[i * 3 + 1];
    int z = indicies[i * 3 + 2];

    EXPECT_EQ((x != y && y != z && x != z), true)
        << "sphere contained triangle with multiple of same index. tri=" << i
        << ", x=" << x << ", y=" << y << ", z=" << z;
    EXPECT_EQ((x < nOfVertices && y < nOfVertices && z < nOfVertices), true)
        << "sphere contained index with out of range vertex";
  }
}
