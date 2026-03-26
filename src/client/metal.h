#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

/**
 * @class material
 * @brief Abstract base class representing how light interacts with a surface.
 *
 * Materials define the scattering behavior of incoming rays when they hit
 * a surface. The scatter() function determines whether a ray is absorbed,
 * reflected, refracted, or diffused.
 *
 * ------------------------------------------------------------
 * Ray interaction model
 * ------------------------------------------------------------
 *
 * A ray hitting a surface produces a new ray according to the material's
 * physical model:
 *
 *      r_scattered = f(r_in, surface_properties)
 *
 * where:
 *
 *      r_in = incoming ray
 *      r_scattered = outgoing ray
 *
 * The material also determines the energy attenuation of the ray:
 *
 *      L_out = attenuation ⊙ L_in
 *
 * where:
 *
 *      attenuation = surface color or reflectance
 *      ⊙ = component-wise multiplication
 *
 * Derived material classes implement specific physical behaviors
 * such as diffuse scattering, reflection, or refraction.
 */
class material {
public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const {
        return false;
    }
};

/**
 * @class lambertian
 * @brief Diffuse material following Lambert's Cosine Law.
 *
 * Lambertian surfaces scatter light uniformly in all directions
 * over the hemisphere above the surface.
 *
 * ------------------------------------------------------------
 * Lambert's Cosine Law
 * ------------------------------------------------------------
 *
 * The reflected intensity depends on the angle between the surface
 * normal and the outgoing direction:
 *
 *      I ∝ cos(θ)
 *
 * where:
 *
 *      θ = angle between outgoing ray and surface normal
 *
 *
 * ------------------------------------------------------------
 * Scatter direction
 * ------------------------------------------------------------
 *
 * The scattered ray direction is approximated by:
 *
 *      scatter_direction = N + random_unit_vector()
 *
 * where:
 *
 *      N = surface normal
 *
 * This generates a random direction in the hemisphere around the normal.
 *
 *
 * ------------------------------------------------------------
 * Attenuation
 * ------------------------------------------------------------
 *
 * The reflected light is scaled by the surface albedo:
 *
 *      L_out = albedo ⊙ L_in
 *
 * where albedo represents the intrinsic color of the material.
 */
class lambertian : public material {
public:
    lambertian(const color& albedo) : albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

/**
 * @class metal
 * @brief Reflective material that models metallic surfaces.
 *
 * Metallic materials reflect incoming rays according to the
 * law of reflection, with optional surface roughness ("fuzz").
 *
 * ------------------------------------------------------------
 * Reflection equation
 * ------------------------------------------------------------
 *
 * The reflected vector is computed as:
 *
 *      R = V − 2(V · N)N
 *
 * where:
 *
 *      V = incoming ray direction
 *      N = surface normal
 *      R = reflected direction
 *
 *
 * ------------------------------------------------------------
 * Surface roughness (fuzz)
 * ------------------------------------------------------------
 *
 * Imperfect reflection is simulated by perturbing the reflection
 * direction using a random vector:
 *
 *      R_fuzzy = normalize(R) + fuzz * random_unit_vector()
 *
 * where:
 *
 *      fuzz ∈ [0,1]
 *
 * Interpretation:
 *
 *      fuzz = 0 → perfect mirror
 *      fuzz = 1 → very rough metal
 *
 *
 * ------------------------------------------------------------
 * Valid reflection condition
 * ------------------------------------------------------------
 *
 * The reflected ray must leave the surface:
 *
 *      dot(R, N) > 0
 *
 * Otherwise the ray is absorbed.
 */
class metal : public material {
public:
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

private:
    color albedo;
    double fuzz;
};

/**
 * @class dielectric
 * @brief Transparent material that transmits light through refraction (e.g., glass, water).
 *
 * Dielectric materials bend light when it crosses the boundary between
 * two media with different refractive indices. The behavior is governed
 * by Snell's Law.
 *
 * ------------------------------------------------------------
 * Snell's Law
 * ------------------------------------------------------------
 *
 * Refraction follows the relationship:
 *
 *      n₁ sin(θ₁) = n₂ sin(θ₂)
 *
 * where:
 *
 *      n₁ = refractive index of incident medium
 *      n₂ = refractive index of transmitted medium
 *      θ₁ = incident angle
 *      θ₂ = refracted angle
 *
 * In the implementation, we use the ratio:
 *
 *      η = n₁ / n₂
 *
 *
 * ------------------------------------------------------------
 * Computing the incident angle
 * ------------------------------------------------------------
 *
 * The cosine of the incident angle is obtained using the dot product:
 *
 *      cosθ = min( dot(-uv, N), 1 )
 *
 * where:
 *
 *      uv = normalized incoming direction
 *      N  = surface normal
 *
 * The sine is derived using the identity:
 *
 *      sin²θ + cos²θ = 1
 *
 * so
 *
 *      sinθ = √(1 − cos²θ)
 *
 *
 * ------------------------------------------------------------
 * Refraction vector
 * ------------------------------------------------------------
 *
 * The refracted ray is decomposed into two components:
 *
 * Perpendicular component:
 *
 *      r⊥ = η (uv + cosθ N)
 *
 * Parallel component:
 *
 *      r∥ = -√(1 - |r⊥|²) N
 *
 * Final refracted ray:
 *
 *      r = r⊥ + r∥
 *
 *
 * ------------------------------------------------------------
 * Total Internal Reflection
 * ------------------------------------------------------------
 *
 * Refraction becomes impossible when Snell's law cannot be satisfied.
 * This occurs when:
 *
 *      η sinθ > 1
 *
 * In this case, the ray undergoes total internal reflection instead of
 * refraction.
 *
 * The reflected direction follows the mirror reflection equation:
 *
 *      R = V − 2(V · N)N
 *
 *
 * ------------------------------------------------------------
 * Refractive index
 * ------------------------------------------------------------
 *
 * The refractive index determines how much light bends when entering
 * the material.
 *
 * Typical values:
 *
 *      air   ≈ 1.0003
 *      water ≈ 1.33
 *      glass ≈ 1.5
 *
 *
 * ------------------------------------------------------------
 * Attenuation
 * ------------------------------------------------------------
 *
 * Ideal dielectrics do not absorb light, so the ray keeps its energy:
 *
 *      attenuation = (1, 1, 1)
 *
 * meaning the color of the ray is unchanged.
 */
class dielectric : public material {
public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;
        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);
        scattered = ray(rec.p, direction);
        return true;
    }
private:
    static double reflectance(double cosine, double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;
};

#endif