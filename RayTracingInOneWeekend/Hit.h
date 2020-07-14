#ifndef HIT_H_
#define HIT_H_
#include "Ray.h"
#include <vector>
#include <memory>

struct Material;

struct HitRecord {
  Point3f p;
  Vec3f normal;
  std::shared_ptr<Material> material;
  float t;
  bool frontFace;

  void setFaceNormal(const Ray &r, const Vec3f &outward) {
    frontFace = dot(r.dir, outward) < 0;
    normal = frontFace ? outward : -outward;
  }
};

struct Hitable {
  virtual bool hit(const Ray &r, float tMin, float tMax,
                   HitRecord &record) const = 0;
};

struct HitList : public Hitable {
  HitList() = default;
  HitList(std::shared_ptr<Hitable> object) { add(object); }

  void clear() { objects.clear(); }

  void add(std::shared_ptr<Hitable> object) { objects.push_back(object); }

  std::vector<std::shared_ptr<Hitable> > objects;

  bool hit(const Ray &r, float tMin, float tMax,
           HitRecord &rec) const override {
    HitRecord tmpRec;
    bool hitAny = false;
    auto closest = tMax;
    for (const auto &object : objects) {
      if (object->hit(r, tMin, closest, tmpRec)) {
        hitAny = true;
        closest = tmpRec.t;
        rec = tmpRec;
      }
    }
    return hitAny;
  }
};

#endif