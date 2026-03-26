#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "rtweekend.h"

class sphere : public hittable {
private:
    point3 center;
    double radius;
    shared_ptr<material> mat;

public:
    sphere(const point3& center, double radius, shared_ptr<material> mat)
    : center(center), radius(std::fmax(0,radius)), mat(mat) {}

    /**
     * @brief Computes the intersection between a ray and a sphere.
     *
     * This function solves the geometric problem of whether a ray intersects a sphere.
     * The intersection is found by substituting the ray equation into the sphere equation
     * and solving the resulting quadratic equation for the ray parameter t.
     *
     * ------------------------------------------------------------
     * 1. Ray equation
     * ------------------------------------------------------------
     *
     * A ray is defined parametrically as:
     *
     *      P(t) = O + tD
     *
     * where:
     *      O = ray origin
     *      D = ray direction
     *      t = distance along the ray
     *      P(t) = point along the ray
     *
     *
     * ------------------------------------------------------------
     * 2. Sphere equation
     * ------------------------------------------------------------
     *
     * A sphere with center C and radius r satisfies:
     *
     *      |P - C|² = r²
     *
     * which means every point on the sphere is exactly r units from its center.
     *
     * ------------------------------------------------------------
     * 3. Substitute the ray into the sphere equation
     * ------------------------------------------------------------
     *
     * Replace P with the ray equation:
     *
     *      |(O + tD) - C|² = r²
     *
     * Rearranging:
     *
     *      |(O - C) + tD|² = r²
     *
     * ------------------------------------------------------------
     * 4. Expand the dot product
     * ------------------------------------------------------------
     *
     * Using |v|² = v·v:
     *
     *      ((O - C) + tD) · ((O - C) + tD) = r²
     *
     * Expanding:
     *
     *      (O - C)·(O - C) + 2t D·(O - C) + t² D·D = r²
     *
     * ------------------------------------------------------------
     * 5. Rearrange into quadratic form
     * ------------------------------------------------------------
     *
     * Bringing everything to one side gives:
     *
     *      (D·D)t² + 2D·(O - C)t + ((O - C)·(O - C) - r²) = 0
     *
     * This matches the standard quadratic form:
     *
     *      at² + bt + c = 0
     *
     * with:
     *
     *      a = D·D
     *      b = 2D·(O - C)
     *      c = (O - C)·(O - C) - r²
     *
     * ------------------------------------------------------------
     * 6. Half-b optimization used in this implementation
     * ------------------------------------------------------------
     *
     * Instead of storing b, the code stores:
     *
     *      h = D·(C - O)
     *
     * which equals:
     *
     *      h = -(D·(O - C))
     *
     * This lets us rewrite the quadratic as:
     *
     *      a t² - 2h t + c = 0
     *
     * The quadratic solution becomes:
     *
     *      t = (h ± √(h² - ac)) / a
     *
     * avoiding the factor of 2 and slightly reducing computation.
     *
     *
     * ------------------------------------------------------------
     * 7. Discriminant
     * ------------------------------------------------------------
     *
     * The discriminant determines if an intersection exists:
     *
     *      Δ = h² - ac
     *
     *      Δ < 0  → no intersection
     *      Δ = 0  → tangent (one intersection)
     *      Δ > 0  → two intersection points
     *
     *
     * ------------------------------------------------------------
     * 8. Choosing the correct intersection
     * ------------------------------------------------------------
     *
     * Two possible roots exist:
     *
     *      t₁ = (h - √Δ) / a
     *      t₂ = (h + √Δ) / a
     *
     * The algorithm chooses the closest valid root within the allowed
     * ray interval (ray_t). If the nearest root is outside the interval,
     * the second root is tested.
     *
     *
     * ------------------------------------------------------------
     * 9. Computing hit information
     * ------------------------------------------------------------
     *
     * Once t is found:
     *
     *      P = O + tD
     *
     * gives the hit point.
     *
     * The surface normal of a sphere is:
     *
     *      N = (P - C) / r
     *
     * which points outward from the sphere.
     *
     * This information is stored in the hit_record for shading,
     * reflection, refraction, and other lighting calculations.
     */
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

        // Ray equation:
        // P(t) = O + tD
        // O = ray origin
        // D = ray direction

        // Sphere equation:
        // |P - C|² = r²
        // C = sphere center
        // r = sphere radius

        vec3 oc = center - r.origin();
        // oc = C - O
        // Vector from ray origin to sphere center

        auto a = r.direction().length_squared();
        // a = D · D
        // First quadratic coefficient

        auto h = dot(r.direction(), oc);
        // h = D · (C - O)
        // Half of the traditional b term (b = 2D·(C-O))

        auto c = oc.length_squared() - radius*radius;
        // c = (C - O) · (C - O) - r²
        // Third quadratic coefficient

        // Quadratic form used:
        // a t² - 2h t + c = 0

        auto discriminant = h*h - a*c;
        // Δ = h² - a c
        // If Δ < 0 → no real solution → ray misses sphere

        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);
        // √Δ

        // Quadratic roots:
        // t = (h ± √Δ) / a

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        // t₁ = (h - √Δ) / a
        // Closest intersection point

        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            // t₂ = (h + √Δ) / a
            // Second intersection (farther side of sphere)

            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        // Store t (distance along the ray)

        rec.p = r.at(rec.t);
        // P = O + tD
        // Actual intersection point in space

        rec.normal = (rec.p - center) / radius;
        // Surface normal
        // N = (P - C) / r

        vec3 outward_normal = (rec.p - center) / radius;
        // Same normal stored separately for face orientation check

        rec.mat = mat;
        // Store material for shading

        rec.set_face_normal(r, outward_normal);
        // Determines if the ray hit the front or back face
        // front_face = dot(D, N) < 0
        // Flips normal if necessary

        return true;
    }

};

#endif