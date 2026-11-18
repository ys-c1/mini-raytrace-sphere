# mini-raytrace-sphere

A small C11 ray tracer with no third-party dependencies. It provides a
sphere/ray intersection routine and a fixed perspective-camera pixel shader
using Lambert diffuse lighting and nearest-hit selection.

## Files

- `src/raytrace.h` - public API
- `src/raytrace.c` - intersection and shading implementation
- `tests/test_raytrace.c` - self-contained test runner
- `Makefile` - build, test, and clean targets

## Build and test

```sh
make test
```

The runner prints one `[PASS]`/`[FAIL]` line per check and exits non-zero if
any check fails.

## API

```c
int ray_sphere(const double *origin, const double *dir, const double *center,
               double radius, double *t_hit, double *normal);
double trace_pixel(double px, double py, int width, int height);
```

`ray_sphere` returns `1` for a hit and `-1` for a miss. On a hit, `t_hit` is
the distance to the nearest positive intersection and `normal` is the unit
outward normal. On a miss, `t_hit` is set to `-1` and `normal` is zeroed.

`trace_pixel` uses a camera at the origin looking down `-Z`, with a square
pixel grid scaled by the image aspect ratio. It tests both spheres in the
fixed scene and keeps the closest valid hit. The returned value is an integer
grayscale sample in `[0, 255]`, with `0` reserved for the background.

## Current repository state

This repository intentionally contains three algorithm-level defects for
debugging practice:

- B1: the quadratic discriminant adds `c` to `dot(oc, oc)` instead of
  subtracting `c`, so the sign is wrong and root classification breaks.
- B2: the hit distance selects the larger/far root instead of the smaller/near
  root.
- B3: the surface normal is left unnormalized, so Lambert values saturate at
  255 and mid-tones are lost.

The accompanying test suite is the correct-behavior suite. In the buggy
state it is expected to report failures.
