#ifndef MESH
#define MESH

#include "hittable.h"
#include "rtweekend.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool hit_triangle(const point3& v0, const point3& v1, const point3& v2,
            const ray& r, double t_min, double t_max, hit_record& rec) {

    const double EPSILON = 1e-8;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    vec3 h = cross(r.direction(), edge2);
    double a = dot(edge1, h);

    if (fabs(a) < EPSILON)
        return false; // paralelo

    double f = 1.0 / a;
    vec3 s = r.origin() - v0;

    double u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(s, edge1);

    double v = f * dot(r.direction(), q);
    if (v < 0.0 || u + v > 1.0)
        return false;

    double t = f * dot(edge2, q);

    if (t < t_min || t > t_max)
        return false;

    rec.t = t;
    rec.p = r.at(t);

    vec3 normal = unit_vector(cross(edge1, edge2));
    rec.set_face_normal(r, normal);

    return true;
}

class mesh : public hittable {
public:
    std::vector<point3> vertices;
    std::vector<int> indices;
    shared_ptr<material> mat;

    mesh(
        const std::vector<point3>& v,
        const std::vector<int>& i,
        shared_ptr<material> m
    ) :
        vertices(v),
        indices(i),
        mat(m)
    {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

        hit_record temp_rec;
        bool hit_anything = false;
        auto closest = ray_t.max;

        for (size_t i = 0; i < indices.size(); i += 3) {

            const point3& v0 = vertices[indices[i]];
            const point3& v1 = vertices[indices[i+1]];
            const point3& v2 = vertices[indices[i+2]];

            if (hit_triangle(v0, v1, v2, r, ray_t.min, closest, temp_rec)) {
                hit_anything = true;
                closest = temp_rec.t;
                rec = temp_rec;
                rec.mat = mat;
            }
        }

        return hit_anything;
    }
};

class transform : public hittable {
public:
    shared_ptr<hittable> object;

    glm::mat4 transform_matrix;
    glm::mat4 inv_matrix;

    transform(
        shared_ptr<hittable> obj,
        const glm::mat4& m
    ) :
        object(obj),
        transform_matrix(m),
        inv_matrix(glm::inverse(m)
    )
    {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

        // transforma o ray para espaço local
        glm::vec4 origin =
            inv_matrix * glm::vec4(
                r.origin().x(),
                r.origin().y(),
                r.origin().z(),
                1.0
            );
        glm::vec4 dir = inv_matrix * glm::vec4(r.direction().x(), r.direction().y(), r.direction().z(), 0.0);

        ray transformed_ray(
            point3(origin.x, origin.y, origin.z),
            vec3(dir.x, dir.y, dir.z)
        );

        if (!object->hit(transformed_ray, ray_t, rec))
            return false;

        // volta pro espaço mundo
        glm::vec4 p = transform_matrix * glm::vec4(rec.p.x(), rec.p.y(), rec.p.z(), 1.0);
        rec.p = point3(p.x, p.y, p.z);

        glm::vec4 normal = transform_matrix * glm::vec4(rec.normal.x(), rec.normal.y(), rec.normal.z(), 0.0);
        rec.normal = unit_vector(vec3(normal.x, normal.y, normal.z));

        return true;
    }
};

#endif