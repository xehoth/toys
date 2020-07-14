#ifndef MATERIAL_H_
#define MATERIAL_H_
#include "Ray.h"
#include "Hit.h"

struct Material {
  virtual bool scatter(const Ray &r, const HitRecord &rec, Color3f &attenuation,
                       Ray &scattered) const = 0;
};

struct Lambertian : public Material {
  Lambertian(const Color3f &a) : albedo(a) {}

  bool scatter(const Ray &r, const HitRecord &rec, Color3f &attenuation,
               Ray &scattered) const override {
    Vec3f scatterDir = rec.normal + randomUnitVector();
    scattered = Ray(rec.p, scatterDir);
    attenuation = albedo;
    return true;
  }

  Color3f albedo;
};

struct Metal : public Material {
  Metal(const Color3f &a, float f) : albedo(a), fuzz(f < 1 ? f : 1) {}

  bool scatter(const Ray &r, const HitRecord &rec, Color3f &attenuation,
               Ray &scattered) const override {
    Vec3f reflected = reflect(normalize(r.dir), rec.normal);
    scattered = Ray(rec.p, reflected + fuzz * randomInUnitSphere());
    attenuation = albedo;
    return dot(scattered.dir, rec.normal) > 0;
  }

  Color3f albedo;
  float fuzz;
};

struct Dielectric : public Material {
  Dielectric(float r) : refIdx(r) {}

  bool scatter(const Ray &r, const HitRecord &rec, Color3f &attenuation,
               Ray &scattered) const override {
    attenuation = Color3f(1.0f, 1.0f, 1.0f);
    float e = rec.frontFace ? 1.0f / refIdx : refIdx;
    Vec3f unitDir = normalize(r.dir);
    float cosTheta = std::min(dot(-unitDir, rec.normal), 1.0f);
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    if (e * sinTheta > 1) {
      Vec3f reflected = reflect(unitDir, rec.normal);
      scattered = Ray(rec.p, reflected);
      return true;
    }
    float reflectProb = schlick(cosTheta, e);
    if (randomFloat() < reflectProb) {
      Vec3f reflected = reflect(unitDir, rec.normal);
      scattered = Ray(rec.p, reflected);
      return true;
    }
    Vec3f refracted = refract(unitDir, rec.normal, e);
    scattered = Ray(rec.p, refracted);
    return true;
  }

  float refIdx;
};

#endif