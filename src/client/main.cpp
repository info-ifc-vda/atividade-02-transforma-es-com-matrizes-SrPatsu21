#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "mesh.h"

int main() {

    // World
    hittable_list world;

    std::vector<point3> vertices = {
        point3(-0.5, -0.5, -1.0),
        point3( 0.5, -0.5, -1.0),
        point3( 0.0, 0.2, -1.0)
    };

    std::vector<int> indices = {
        0,1,2
    };

    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left = make_shared<dielectric>(1.50);
    auto material_bubble = make_shared<dielectric>(1.00 / 1.50);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);

    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    // world.add(make_shared<sphere>(point3( 0.0, 0.0, -1.2), 0.5, material_center));
    // world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    // world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.4, material_bubble));
    // world.add(make_shared<sphere>(point3( 1.0, 0.0, -1.0), 0.5, material_right));

    auto my_mesh = make_shared<mesh>(vertices, indices, material_center);

    //scale
    float Sx=1.5, Sy=1.5, Sz=1.5;
    glm::mat4 scale = glm::mat4(
        Sx, 0,  0,  0,
        0,  Sy, 0,  0,
        0,  0,  Sz, 0,
        0,  0,  0,  1
    );
    //rotate
    float thetay = glm::radians(10.0f);
    float thetaz = glm::radians(50.0f);
    float c = cos(thetay);
    float s = sin(thetay);
    glm::mat4 rotateY = glm::mat4(
        c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1
    );
    c = cos(thetaz);
    s = sin(thetaz);
    glm::mat4 rotateZ = glm::mat4(
        c, -s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
    //shear
    float theta_xy = glm::radians(10.0f),
        theta_xz = glm::radians(0.0f),
        theta_yz = glm::radians(-20.0f);

    float k_xy = tan(theta_xy);
    float k_xz = tan(theta_xz);
    float k_yz = tan(theta_yz);

    float sh_xy = k_xy;
    float sh_yx = 0;
    float sh_xz = k_xz;
    float sh_zx = 0;
    float sh_yz = k_yz;
    float sh_zy = 0;

    glm::mat4 shear = glm::mat4(
        1, sh_yx, sh_zx, 0,
        sh_xy, 1, sh_zy, 0,
        sh_xz, sh_yz, 1, 0,
        0, 0, 0, 1
    );
    //translate
    glm::vec3 tl = {2.0, 0.0, -1.0};
    glm::mat4 translate = glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        tl.x, tl.y, tl.z, 1
    );

    glm::mat4 T = translate * rotateY * rotateZ * shear * scale;
    world.add(make_shared<transform>(my_mesh, T));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 960; // 1920;
    cam.samples_per_pixel = 16;
    cam.max_depth = 10;

    cam.vfov = 60;
    cam.lookfrom = point3(0, 0, 0);
    cam.lookat = point3(0, 0, -1);
    cam.vup = vec3(0,1,0);

    cam.defocus_angle = 0.0;
    cam.focus_dist = 3.4;

    cam.render(world);
}