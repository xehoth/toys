#ifndef CAMERA_H_
#define CAMERA_H_
#include "Math.h"
#include "Ray.h"

class Camera {
 public:
  Camera(Point3f lookfrom, Point3f lookat, Vec3f up, float fov,
         float aspectRatio, float aperture, float focusDis) {
    float theta = fov * PI / 180.0f;
    float h = tan(theta / 2);
    float viewportHeight = 2.0f * h;
    float viewportWidth = aspectRatio * viewportHeight;
    float focalLength = 1.0f;

    Vec3f w = normalize(lookfrom - lookat);
    Vec3f u = normalize(cross(up, w));
    Vec3f v = cross(w, u);

    origin = lookfrom;
    horizontal = focusDis * viewportWidth * u;
    vertical = focusDis * viewportHeight * v;
    lowerLeft = origin - horizontal / 2.0f - vertical / 2.0f - w * focusDis;
    lensRadius = aperture / 2;
  }

  Ray getRay(float s, float t) const {
    Vec3f r = lensRadius * randomInUnitDisk();
    Vec3f offset = u * r.x + v * r.y;
    return Ray(origin + offset,
               lowerLeft + s * horizontal + t * vertical - origin - offset);
  }

 private:
  Point3f origin;
  Point3f lowerLeft;
  Vec3f horizontal;
  Vec3f vertical;
  Vec3f u, v, w;
  float lensRadius;
};
#endif