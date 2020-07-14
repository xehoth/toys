#ifndef RAY_H_
#define RAY_H_
#include "Math.h"

struct Ray {
  Ray() = default;
  Ray(const Point3f &origin, const Vec3f &dir) : origin(origin), dir(dir) {}

  Point3f at(float t) const { return origin + dir * t; }

  Point3f origin;
  Vec3f dir;
};
#endif