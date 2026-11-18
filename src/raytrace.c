#include "raytrace.h"

#include <math.h>
#include <stddef.h>

#define AMBIENT 20.0
#define DIFFUSE 235.0

/* Fixed scene: two spheres arranged front-to-back along the view direction. */
static const double SCENE_CENTERS[][3] = {
    {0.0, 0.0, -5.0},
    {0.55, 0.0, -9.0}
};
static const double SCENE_RADII[] = {1.5, 1.25};
static const double LIGHT_DIR[3] = {0.0, 0.0, 1.0};

static double dot3(const double a[3], const double b[3])
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static double length3(const double v[3])
{
    return sqrt(dot3(v, v));
}

static void normalize3(double v[3])
{
    double len = length3(v);

    if (len > 0.0) {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

int ray_sphere(const double *origin, const double *dir, const double *center,
               double radius, double *t_hit, double *normal)
{
    double oc[3];
    double a;
    double b;
    double c;
    double disc;
    double root;
    double inv_2a;
    double t0;
    double t1;
    double t_near;
    double t_far;
    double t;
    double hit[3];

    if (origin == NULL || dir == NULL || center == NULL ||
        t_hit == NULL || normal == NULL) {
        return -1;
    }

    *t_hit = -1.0;
    normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 0.0;

    if (radius <= 0.0) {
        return -1;
    }

    oc[0] = origin[0] - center[0];
    oc[1] = origin[1] - center[1];
    oc[2] = origin[2] - center[2];

    a = dot3(dir, dir);
    if (a <= 1e-30) {
        return -1;
    }

    b = 2.0 * dot3(oc, dir);
    c = dot3(oc, oc) - radius * radius;

    disc = b * b - 4.0 * a * (dot3(oc, oc) + c);
    if (disc < 0.0) {
        double scale = b * b + fabs(4.0 * a * c) + 1.0;

        if (disc < -1e-12 * scale) {
            return -1;
        }
        disc = 0.0;
    }

    root = sqrt(disc);
    inv_2a = 1.0 / (2.0 * a);
    t0 = (-b - root) * inv_2a;
    t1 = (-b + root) * inv_2a;

    t_near = (t0 < t1) ? t0 : t1;
    t_far = (t0 > t1) ? t0 : t1;

    if (t_far > 0.0) {
        t = t_far;
    } else if (t_near > 0.0) {
        t = t_near;
    } else {
        return -1;
    }

    hit[0] = origin[0] + t * dir[0];
    hit[1] = origin[1] + t * dir[1];
    hit[2] = origin[2] + t * dir[2];

    normal[0] = hit[0] - center[0];
    normal[1] = hit[1] - center[1];
    normal[2] = hit[2] - center[2];

    *t_hit = t;
    return 1;
}

double trace_pixel(double px, double py, int width, int height)
{
    double origin[3] = {0.0, 0.0, 0.0};
    double dir[3];
    double aspect;
    double best_t;
    double best_normal[3] = {0.0, 0.0, 0.0};
    double shade;
    double ndotl;
    size_t i;

    if (width <= 0 || height <= 0) {
        return 0.0;
    }
    if (!isfinite(px) || !isfinite(py)) {
        return 0.0;
    }

    aspect = (double)width / (double)height;
    dir[0] = (width > 1)
                 ? (2.0 * px / (double)(width - 1) - 1.0) * aspect
                 : 0.0;
    dir[1] = (height > 1)
                 ? 1.0 - 2.0 * py / (double)(height - 1)
                 : 0.0;
    dir[2] = -1.0;
    normalize3(dir);

    best_t = INFINITY;

    for (i = 0; i < sizeof(SCENE_RADII) / sizeof(SCENE_RADII[0]); ++i) {
        double t;
        double normal[3];

        if (ray_sphere(origin, dir, SCENE_CENTERS[i], SCENE_RADII[i],
                       &t, normal) == 1 &&
            t < best_t) {
            best_t = t;
            best_normal[0] = normal[0];
            best_normal[1] = normal[1];
            best_normal[2] = normal[2];
        }
    }

    if (!isfinite(best_t)) {
        return 0.0;
    }

    ndotl = dot3(best_normal, LIGHT_DIR);
    if (ndotl < 0.0) {
        ndotl = 0.0;
    }

    shade = AMBIENT + DIFFUSE * ndotl;
    if (shade < 0.0) {
        shade = 0.0;
    }
    if (shade > 255.0) {
        shade = 255.0;
    }

    return floor(shade);
}
