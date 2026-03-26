#ifndef VEC3_H
#define VEC3_H

#include "rtweekend.h"

class vec3 {
public:
    double e[3];

    vec3() : e{0,0,0} {}
    vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    static vec3 random() {
        return vec3(random_double(), random_double(), random_double());
    }

    static vec3 random(double min, double max) {
        return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
    }

    vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*=(double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator/=(double t) {
        return *this *= 1/t;
    }

    double length() const {
        return std::sqrt(length_squared());
    }

    double length_squared() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }

    bool near_zero() const {
        // Return true if the vector is close to zero in all dimensions.
        auto s = 1e-8;
        // absolute float: fabs(double) fabsf(float) fabsl(long double)
        // fabs is tradition carried into early C++
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point3 = vec3;


// Vector Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) {
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
    return (1/t) * v;
}

// Dot product between two vectors
// Formula:
//
// u · v = uₓvₓ + uᵧvᵧ + u_zv_z
//
// Geometric meaning:
// u · v = |u||v|cos(θ)
// where θ is the angle between the vectors.
// Used to measure alignment between two directions.
inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
         + u.e[1] * v.e[1]
         + u.e[2] * v.e[2];
}

/**
 * Cross product between two vectors
 *  Formula:
 *
 * u × v = (uᵧv𝔷 − u𝔷vᵧ, u𝔷vₓ − uₓv𝔷, uₓvᵧ − uᵧvₓ)
 *
 * Geometric meaning:
 * Result is a vector perpendicular to both u and v.
 *
 * Magnitude:
 *
 * |u × v| = |u||v| sin(θ)
 *
 * Used heavily for surface normals and orientation.
*/
inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

/**
 * Normalize a vector (convert to unit length)
 *
 * Formula:
 *
 * v̂ = v / |v|
 *
 * where
 *
 * |v| = √(v · v)
 *
 * Result:
 * |v̂| = 1
 *
 */
inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

/**
 * @brief Generates a random point inside the unit disk on the XY plane.
 *
 * The unit disk is defined as all points (x, y) such that:
 *
 *      x² + y² < 1
 *
 * This function uses rejection sampling:
 *
 * 1. Generate a random point in the square:
 *
 *      x ∈ [-1, 1],  y ∈ [-1, 1]
 *
 * 2. Accept the point only if it lies inside the unit circle:
 *
 *      x² + y² < 1
 *
 * 3. Otherwise, discard it and try again.
 *
 *
 * ------------------------------------------------------------
 * Mathematical interpretation
 * ------------------------------------------------------------
 *
 * The function ensures:
 *
 *      |p|² = x² + y² < 1
 *
 * which means the point lies strictly inside the unit disk.
 *
 *
 * ------------------------------------------------------------
 * Why z = 0?
 * ------------------------------------------------------------
 *
 * The disk lies in the XY plane:
 *
 *      p = (x, y, 0)
 *
 * This is useful for:
 *  - depth of field (camera lens simulation)
 *  - sampling circular areas
 *
 *
 * ------------------------------------------------------------
 * Uniform distribution
 * ------------------------------------------------------------
 *
 * Rejection sampling produces a uniform distribution over the disk,
 * meaning every point inside the circle has equal probability.
 *
 */
inline vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() < 1)
        return p;
    }
}

/**
 * Generate a random vector uniformly distributed on the unit sphere.
 *
 * Method:
 * 1. Random point inside cube [-1,1]³
 * 2. Reject points outside the unit sphere
 * 3. Normalize the vector
 *
 * Condition:
 *
 * 0 < |p|² ≤ 1
 *
 * Returned vector:
 *
 * p̂ = p / |p|
 */
inline vec3 random_unit_vector() {
    while (true) {
        auto p = vec3::random(-1,1);
        auto lensq = p.length_squared();
        if (1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}

/**
 * Refraction using Snell's Law.
 *
 * Snell's law:
 *
 * n₁ sin(θ₁) = n₂ sin(θ₂)
 *
 * The refracted ray is decomposed into two components:
 *
 * r_out_perp     = η (uv + cosθ n)
 * r_out_parallel = -√(1 - |r_out_perp|²) n
 *
 * where:
 *
 * η = etai_over_etat = n₁ / n₂
 *
 * Final refracted vector:
 *
 * r = r_out_perp + r_out_parallel
 */
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

/**
 * Generate a random direction on the hemisphere defined by a normal.
 *
 * Method:
 * 1. Pick random direction on unit sphere
 * 2. Keep it if it points in the same hemisphere
 *
 * Condition:
 *
 * dot(v, normal) > 0
 *
 * If not, flip the vector.
 */
inline vec3 random_on_hemisphere(const vec3& normal) {
    vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

/**
 * Reflection of vector v around normal n.
 *
 * Formula:
 *
 * r = v − 2(v · n)n
 *
 * Derived from projecting v onto the normal
 * and subtracting twice that component.
 */
inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2*dot(v,n)*n;
}

#endif