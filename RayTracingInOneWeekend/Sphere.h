#ifndef SPHERE_H_
#define SPHERE_H_
#include "Hit.h"

struct Sphere : public Hitable {
  Sphere() = default;
  Sphere(const Point3f &center, float r, std::shared_ptr<Material> material)
      : center(center), radius(r), material(material) {}

  bool hit(const Ray &r, float tMin, float tMax,
           HitRecord &rec) const override {
    Vec3f oc = r.origin - center;
    auto a = r.dir.norm2();
    auto halfB = dot(oc, r.dir);
    auto c = oc.norm2() - radius * radius;
    auto delta = halfB * halfB - a * c;
    if (delta > 0) {
      delta = sqrt(delta);
      auto tmp = (-halfB - delta) / a;
      if (tmp < tMax && tmp > tMin) {
        rec.t = tmp;
        rec.p = r.at(rec.t);
        Vec3f outward = (rec.p - center) / radius;
        rec.setFaceNormal(r, outward);
        rec.material = material;
        return true;
      }
      tmp = (-halfB + delta) / a;
      if (tmp < tMax && tmp > tMin) {
        rec.t = tmp;
        rec.p = r.at(rec.t);
        Vec3f outward = (rec.p - center) / radius;
        rec.setFaceNormal(r, outward);
        rec.material = material;
        return true;
      }
    }
    return false;
  }

  Point3f center;
  float radius;
  std::shared_ptr<Material> material;
};
#endif