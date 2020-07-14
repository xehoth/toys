#ifndef MATH_H_
#define MATH_H_
#include <cmath>
#include <iostream>
#include <random>

inline float randomFloat() { return rand() / (RAND_MAX + 1.0f); }

inline float randomFloat(float min, float max) {
  return min + (max - min) * randomFloat();
}

inline float clamp(float x, float min, float max) {
  return x < min ? min : (x > max ? max : x);
}

template <typename T>
struct Vec3 {
  union {
    struct {
      T x;
      T y;
      T z;
    };
    struct {
      T r;
      T g;
      T b;
    };
    T a[3];
  };

  Vec3() : a() {}
  Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

  Vec3 operator-() const { return {-x, -y, -z}; }
  Vec3 &operator+=(const Vec3 &o) {
    x += o.x;
    y += o.y;
    z += o.z;
    return *this;
  }
  Vec3 &operator-=(const Vec3 &o) {
    x -= o.x;
    y -= o.y;
    z -= o.z;
    return *this;
  }
  Vec3 operator+(const Vec3 &o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vec3 operator-(const Vec3 &o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vec3 operator*(T t) const { return {x * t, y * t, z * t}; }
  Vec3 &operator*=(T t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
  }
  Vec3 operator/(T t) const { return *this * (1 / t); }
  Vec3 &operator/=(T t) { return *this *= 1 / t; }

  T length() const { return sqrt(norm2()); }
  T norm() const { return sqrt(norm2()); }
  T length2() const { return x * x + y * y + z * z; }
  T norm2() const { return x * x + y * y + z * z; }

  T &operator[](int i) { return a[i]; }
  T operator[](int i) const { return a[i]; }

  T dot(const Vec3 &o) const { return x * o.x + y * o.y + z * o.z; }
  Vec3 cross(const Vec3 &o) const {
    return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
  }

  Vec3 &normalize() { *this /= norm(); }

  Vec3 normalized() const { return *this / norm(); }
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const Vec3<T> &c) {
  os << '(' << c.r << ", " << c.g << ", " << c.b << ')';
  return os;
}

template <typename T>
inline Vec3<T> operator*(const Vec3<T> &a, const Vec3<T> &b) {
  return {a.x * b.x, a.y * b.y, a.z * b.z};
}

template <typename T>
inline Vec3<T> operator*(T t, const Vec3<T> &v) {
  return {t * v.x, t * v.y, t * v.z};
}

template <typename T>
inline T dot(const Vec3<T> &a, const Vec3<T> &b) {
  return a.dot(b);
}

template <typename T>
inline Vec3<T> cross(const Vec3<T> &a, const Vec3<T> &b) {
  return a.cross(b);
}

template <typename T>
inline Vec3<T> normalize(const Vec3<T> &v) {
  return v.normalized();
}

using Vec3f = Vec3<float>;
using Color3f = Vec3f;
using Point3f = Vec3f;

inline Vec3f randomVec3f() {
  return Vec3f(randomFloat(), randomFloat(), randomFloat());
}

inline Vec3f randomVec3f(float min, float max) {
  return Vec3f(randomFloat(min, max), randomFloat(min, max),
               randomFloat(min, max));
}

Vec3f randomInUnitSphere() {
  for (;;) {
    auto p = randomVec3f(-1, 1);
    if (p.norm2() >= 1) continue;
    return p;
  }
}

const float PI = acos(-1);

Vec3f randomUnitVector() {
  float a = randomFloat(0, 2 * PI);
  float z = randomFloat(-1, 1);
  float r = sqrt(1 - z * z);
  return Vec3f(r * cos(a), r * sin(a), z);
}

Vec3f randomInHemisphere(const Vec3f &normal) {
  Vec3f inUnitSphere = randomInUnitSphere();
  return dot(inUnitSphere, normal) > 0 ? inUnitSphere : -inUnitSphere;
}

inline Vec3f reflect(const Vec3f &v, const Vec3f &n) {
  return v - 2 * dot(v, n) * n;
}

inline Vec3f refract(const Vec3f &uv, const Vec3f &n, float e) {
  float cosTheta = dot(-uv, n);
  Vec3f r1 = e * (uv + cosTheta * n);
  Vec3f r2 = -sqrt(1 - r1.norm2()) * n;
  return r1 + r2;
}

inline float schlick(float cosine, float refIdx) {
  float r0 = (1 - refIdx) / (1 + refIdx);
  r0 *= r0;
  return r0 + (1 - r0) * pow(1 - cosine, 5);
}

Vec3f randomInUnitDisk() {
  for (;;) {
    auto p = Vec3f(randomFloat(-1, 1), randomFloat(-1, 1), 0);
    if (p.norm2() >= 1) continue;
    return p;
  }
}

#endif