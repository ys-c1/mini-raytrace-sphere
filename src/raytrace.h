#ifndef RAYTRACE_H
#define RAYTRACE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Intersect a ray with a sphere.
 *
 * origin: ray origin (3 doubles)
 * dir:    ray direction (3 doubles); it is normalized internally
 * center: sphere center (3 doubles)
 * radius: sphere radius, must be positive
 * t_hit:  receives the distance to the nearest valid hit, or -1 on a miss
 * normal: receives the unit outward surface normal on a hit
 *
 * Returns 1 on a hit and -1 on a miss or invalid input.
 */
int ray_sphere(const double *origin, const double *dir, const double *center,
               double radius, double *t_hit, double *normal);

/*
 * Shade one pixel of the fixed perspective-camera scene.
 * Pixel coordinates are zero-based and lie in [0, width - 1] and
 * [0, height - 1]. The return value is an integer grayscale in [0, 255].
 */
double trace_pixel(double px, double py, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
