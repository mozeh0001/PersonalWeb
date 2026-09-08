#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "scene.h"

// Basic Vector Math
struct Vec3 {
    float x, y, z;

    Vec3 operator-(const Vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vec3 operator+(const Vec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }

    float dot(const Vec3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec3 normalize() const {
        float length = std::sqrt(x * x + y * y + z * z);

        // prevents NaN if vector length is 0
        if (length < 1e-6f) return {0, 0, 0};

        return { x / length, y / length, z / length };
    }
};

// Ray + Hit Info
struct Ray {
    Vec3 origin;
    Vec3 direction;
};

struct HitInfo {
    float t;        // distance along ray
    int objectId;   // object ID from scene
    Vec3 point;     // hit position
    Vec3 normal;    // surface normal
};

// Helper: reject bad t values
inline bool isValidT(float t) {
    return std::isfinite(t) && t > 1e-4f && t < 1000.0f;
}

// Sphere Intersection
inline bool intersectSphere(const Ray& ray, Vec3 center, float radius, float& t) {
    Vec3 oc = ray.origin - center;

    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float sqrtDisc = std::sqrt(discriminant);

    float t0 = (-b - sqrtDisc) / (2.0f * a);
    float t1 = (-b + sqrtDisc) / (2.0f * a);

    if (isValidT(t0)) {
        t = t0;
        return true;
    }

    if (isValidT(t1)) {
        t = t1;
        return true;
    }

    return false;
}

// Plane Intersection
inline bool intersectPlane(const Ray& ray, const Plane& plane, float& t) {
    Vec3 p0 = { (float)plane.px, (float)plane.py, (float)plane.pz };
    Vec3 n  = { (float)plane.nx, (float)plane.ny, (float)plane.nz };

    float denom = ray.direction.dot(n);

    // ray is parallel to plane
    if (fabs(denom) < 1e-6f) return false;

    t = (p0 - ray.origin).dot(n) / denom;
    return isValidT(t);
}

// Box Intersection: Axis-Aligned Bounding Box slab method
inline bool intersectBox(const Ray& ray, const Box& box, float& t) {
    Vec3 min = {
        (float)(box.cx - box.sx / 2),
        (float)(box.cy - box.sy / 2),
        (float)(box.cz - box.sz / 2)
    };

    Vec3 max = {
        (float)(box.cx + box.sx / 2),
        (float)(box.cy + box.sy / 2),
        (float)(box.cz + box.sz / 2)
    };

    float tmin = -1e9f;
    float tmax =  1e9f;

    // X slab
    if (fabs(ray.direction.x) < 1e-6f) {
        if (ray.origin.x < min.x || ray.origin.x > max.x) return false;
    } else {
        float tx1 = (min.x - ray.origin.x) / ray.direction.x;
        float tx2 = (max.x - ray.origin.x) / ray.direction.x;
        tmin = std::max(tmin, std::min(tx1, tx2));
        tmax = std::min(tmax, std::max(tx1, tx2));
    }

    // Y slab
    if (fabs(ray.direction.y) < 1e-6f) {
        if (ray.origin.y < min.y || ray.origin.y > max.y) return false;
    } else {
        float ty1 = (min.y - ray.origin.y) / ray.direction.y;
        float ty2 = (max.y - ray.origin.y) / ray.direction.y;
        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));
    }

    // Z slab
    if (fabs(ray.direction.z) < 1e-6f) {
        if (ray.origin.z < min.z || ray.origin.z > max.z) return false;
    } else {
        float tz1 = (min.z - ray.origin.z) / ray.direction.z;
        float tz2 = (max.z - ray.origin.z) / ray.direction.z;
        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));
    }

    if (tmax < tmin) return false;

    // if ray starts inside box, tmin may be negative, so use tmax
    float candidateT = isValidT(tmin) ? tmin : tmax;

    if (!isValidT(candidateT)) return false;

    t = candidateT;
    return true;
}

// Cylinder Intersection: vertical Y-axis cylinder, side surface only
inline bool intersectCylinder(const Ray& ray, const Cylinder& cyl, float& t) {
    Vec3 center = { (float)cyl.cx, (float)cyl.cy, (float)cyl.cz };
    Vec3 oc = ray.origin - center;

    float a = ray.direction.x * ray.direction.x +
              ray.direction.z * ray.direction.z;

    // ray is vertical / parallel to cylinder axis
    if (fabs(a) < 1e-6f) return false;

    float b = 2.0f * (oc.x * ray.direction.x + oc.z * ray.direction.z);
    float c = oc.x * oc.x + oc.z * oc.z - (float)cyl.radius * (float)cyl.radius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0) return false;

    float sqrtDisc = std::sqrt(discriminant);

    float t0 = (-b - sqrtDisc) / (2.0f * a);
    float t1 = (-b + sqrtDisc) / (2.0f * a);

    // try near hit first, then far hit
    float candidates[2] = { t0, t1 };

    for (float candidateT : candidates) {
        if (!isValidT(candidateT)) continue;

        float y = ray.origin.y + candidateT * ray.direction.y;

        // cylinder exists from cyl.cy to cyl.cy + height
        if (y >= cyl.cy && y <= cyl.cy + cyl.height) {
            t = candidateT;
            return true;
        }
    }

    return false;
}

// Main Scene Intersection - Magen
inline bool intersectScene(const Ray& ray, const Scene& scene, HitInfo& hit) {
    bool hitSomething = false;
    float closestT = 1e9f;

    // Planes
    for (const auto& p : scene.planes) {
        float t;
        if (intersectPlane(ray, p, t)) {
            if (t < closestT) {
                closestT = t;
                hitSomething = true;

                hit.t = t;
                hit.objectId = p.id;
                hit.point = ray.origin + ray.direction * t;
                hit.normal = { (float)p.nx, (float)p.ny, (float)p.nz };
            }
        }
    }

    // Spheres
    for (const auto& s : scene.spheres) {
        Vec3 center = { (float)s.cx, (float)s.cy, (float)s.cz };

        float t;
        if (intersectSphere(ray, center, (float)s.radius, t)) {
            if (t < closestT) {
                closestT = t;
                hitSomething = true;

                hit.t = t;
                hit.objectId = s.id;
                hit.point = ray.origin + ray.direction * t;
                hit.normal = (hit.point - center).normalize();
            }
        }
    }


    // Boxes
    for (const auto& b : scene.boxes) {
        float t;
        if (intersectBox(ray, b, t)) {
            if (t < closestT) {
                closestT = t;
                hitSomething = true;

                hit.t = t;
                hit.objectId = b.id;
                hit.point = ray.origin + ray.direction * t;

                // Approximate box normal by checking closest face
                Vec3 min = {
                    (float)(b.cx - b.sx / 2),
                    (float)(b.cy - b.sy / 2),
                    (float)(b.cz - b.sz / 2)
                };

                Vec3 max = {
                    (float)(b.cx + b.sx / 2),
                    (float)(b.cy + b.sy / 2),
                    (float)(b.cz + b.sz / 2)
                };

                float dxMin = fabs(hit.point.x - min.x);
                float dxMax = fabs(hit.point.x - max.x);
                float dyMin = fabs(hit.point.y - min.y);
                float dyMax = fabs(hit.point.y - max.y);
                float dzMin = fabs(hit.point.z - min.z);
                float dzMax = fabs(hit.point.z - max.z);

                float smallest = dxMin;
                hit.normal = {-1, 0, 0};

                if (dxMax < smallest) { smallest = dxMax; hit.normal = { 1, 0, 0}; }
                if (dyMin < smallest) { smallest = dyMin; hit.normal = { 0,-1, 0}; }
                if (dyMax < smallest) { smallest = dyMax; hit.normal = { 0, 1, 0}; }
                if (dzMin < smallest) { smallest = dzMin; hit.normal = { 0, 0,-1}; }
                if (dzMax < smallest) { smallest = dzMax; hit.normal = { 0, 0, 1}; }
            }
        }
    }

    // Cylinders
    for (const auto& c : scene.cylinders) {
        float t;
        if (intersectCylinder(ray, c, t)) {
            if (t < closestT) {
                closestT = t;
                hitSomething = true;

                hit.t = t;
                hit.objectId = c.id;
                hit.point = ray.origin + ray.direction * t;

                // Side surface normal
                Vec3 centerXZ = { (float)c.cx, 0.0f, (float)c.cz };
                Vec3 hitXZ = { hit.point.x, 0.0f, hit.point.z };
                hit.normal = (hitXZ - centerXZ).normalize();
            }
        }
    }

    return hitSomething;
}

#endif