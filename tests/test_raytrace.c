#include "raytrace.h"

#include <math.h>
#include <stdio.h>

static int checks = 0;
static int failures = 0;

static void check(int condition, const char *name)
{
    ++checks;
    if (condition) {
        printf("[PASS] %s\n", name);
    } else {
        ++failures;
        printf("[FAIL] %s\n", name);
    }
}

static double norm3(const double v[3])
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static void test_direct_center(void)
{
    const double origin[3] = {2.0, 3.0, 4.0};
    const double dir[3] = {0.0, 0.0, -1.0};
    const double center[3] = {2.0, 3.0, -1.0};
    double t = -100.0;
    double normal[3] = {0.0, 0.0, 0.0};
    int rc;

    rc = ray_sphere(origin, dir, center, 2.0, &t, normal);

    check(rc == 1, "direct ray: sphere is hit");
    check(isfinite(t) && fabs(t - 3.0) < 1e-9,
          "direct ray: t_hit equals the near-hit distance");
    check(fabs(norm3(normal) - 1.0) < 1e-9,
          "direct ray: hit normal is unit length");
    check(normal[0] * dir[0] + normal[1] * dir[1] + normal[2] * dir[2] <
              -0.999999,
          "direct ray: normal points against the incoming ray");
    check(fabs(normal[0]) < 1e-12 && fabs(normal[1]) < 1e-12 &&
              normal[2] > 0.999999,
          "direct ray: normal faces the camera");
}

static void test_tangent(void)
{
    const double origin[3] = {0.0, 1.0, 0.0};
    const double dir[3] = {0.0, 0.0, -1.0};
    const double center[3] = {0.0, 0.0, -5.0};
    double t = -100.0;
    double normal[3] = {0.0, 0.0, 0.0};
    int rc;

    rc = ray_sphere(origin, dir, center, 1.0, &t, normal);

    check(rc == 1 || rc == -1,
          "tangent ray: returns an explicit hit or miss without crashing");
    check(isfinite(t), "tangent ray: t_hit is finite");
    check(rc != 1 || fabs(t - 5.0) < 1e-7,
          "tangent ray: hit distance is near the tangency point");
    check(rc != 1 || fabs(norm3(normal) - 1.0) < 1e-7,
          "tangent ray: hit normal is unit length");
}

static void test_miss(void)
{
    const double origin[3] = {0.0, 2.0, 1.0};
    const double dir[3] = {0.0, 0.0, -1.0};
    const double center[3] = {0.0, 0.0, -5.0};
    double t = 0.0;
    double normal[3] = {0.0, 0.0, 0.0};

    check(ray_sphere(origin, dir, center, 1.0, &t, normal) == -1,
          "miss ray: returns -1");
}

static void test_sphere_behind_camera(void)
{
    const double origin[3] = {0.0, 0.0, 0.0};
    const double dir[3] = {0.0, 0.0, -1.0};
    const double center[3] = {0.0, 0.0, 5.0};
    double t = 0.0;
    double normal[3] = {0.0, 0.0, 0.0};

    check(ray_sphere(origin, dir, center, 1.0, &t, normal) == -1,
          "sphere behind camera: returns -1");
}

static void test_two_spheres_nearest(void)
{
    const double origin[3] = {0.0, 0.0, 0.0};
    const double dir[3] = {0.0, 0.0, -1.0};
    const double front_center[3] = {0.5, 0.0, -4.0};
    const double back_center[3] = {-0.5, 0.0, -8.0};
    const double front_t_expected = 4.0 - sqrt(3.0) / 2.0;
    const double back_t_expected = 8.0 - sqrt(3.0) / 2.0;
    double front_t = -1.0;
    double back_t = -1.0;
    double front_normal[3] = {0.0, 0.0, 0.0};
    double back_normal[3] = {0.0, 0.0, 0.0};
    int front_hit;
    int back_hit;
    int nearest_is_front;

    front_hit = ray_sphere(origin, dir, front_center, 1.0,
                           &front_t, front_normal);
    back_hit = ray_sphere(origin, dir, back_center, 1.0,
                          &back_t, back_normal);

    check(front_hit == 1, "two spheres: front sphere is hit");
    check(back_hit == 1, "two spheres: back sphere is hit");
    check(fabs(front_t - front_t_expected) < 1e-9,
          "two spheres: front near-hit distance is correct");
    check(fabs(back_t - back_t_expected) < 1e-9,
          "two spheres: back near-hit distance is correct");

    nearest_is_front = (front_hit == 1 && back_hit == 1 &&
                        front_t < back_t);
    check(nearest_is_front, "two spheres: nearest hit is the front sphere");
    check(front_normal[0] < -0.1 && front_normal[2] > 0.1,
          "two spheres: front normal identifies the front surface");
    check(back_normal[0] > 0.1 && back_normal[2] > 0.1,
          "two spheres: back normal identifies the back surface");
    check(fabs(norm3(front_normal) - 1.0) < 1e-9,
          "two spheres: nearest-hit normal is unit length");
}

static void test_trace_pixel(void)
{
    const int width = 301;
    const int height = 301;
    double center;
    double adjacent;
    double mid;
    double outer;
    double corner_tl;
    double corner_tr;
    double corner_bl;
    double corner_br;
    int in_range = 1;
    int x;
    int y;

    center = trace_pixel(150.0, 150.0, width, height);
    adjacent = trace_pixel(151.0, 150.0, width, height);
    mid = trace_pixel(170.0, 150.0, width, height);
    outer = trace_pixel(190.0, 150.0, width, height);
    corner_tl = trace_pixel(0.0, 0.0, width, height);
    corner_tr = trace_pixel((double)(width - 1), 0.0, width, height);
    corner_bl = trace_pixel(0.0, (double)(height - 1), width, height);
    corner_br = trace_pixel((double)(width - 1),
                            (double)(height - 1), width, height);

    check(fabs(center - 255.0) < 1e-9,
          "trace_pixel: center pixel is full bright");
    check(adjacent < center,
          "trace_pixel: center is brighter than an adjacent pixel");
    check(mid > 0.0 && mid < 255.0,
          "trace_pixel: mid-region has an intermediate gray level");
    check(mid > outer && outer > 0.0,
          "trace_pixel: Lambert shading falls off away from the center");

    check(corner_tl == 0.0, "trace_pixel: top-left corner is background");
    check(corner_tr == 0.0, "trace_pixel: top-right corner is background");
    check(corner_bl == 0.0, "trace_pixel: bottom-left corner is background");
    check(corner_br == 0.0, "trace_pixel: bottom-right corner is background");

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            double value = trace_pixel((double)x, (double)y, width, height);

            if (value < 0.0 || value > 255.0) {
                in_range = 0;
            }
        }
    }
    check(in_range, "trace_pixel: every pixel is in the 0..255 range");
}

int main(void)
{
    printf("raytrace test suite\n");

    test_direct_center();
    test_tangent();
    test_miss();
    test_sphere_behind_camera();
    test_two_spheres_nearest();
    test_trace_pixel();

    printf("\nchecks: %d, failures: %d\n", checks, failures);
    return (failures == 0) ? 0 : 1;
}
