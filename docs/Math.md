# Math

## Mat4

`#include "LiLib/Math/Mat4.h"`

A column-major 4x4 matrix, laid out so it can be uploaded to shaders directly through `Data()`. A default-constructed `Mat4` is the identity.

```cpp
#include "LiLib/Math/Mat4.h"

const Vec3 position{1.0f, 0.0f, 0.0f};
const Vec3 eye{0.0f, 2.0f, 5.0f};

const Mat4 model = Mat4::Translation(position) * Mat4::RotationY(0.5f) * Mat4::Scaling({2.0f, 2.0f, 2.0f});
const Mat4 view = Mat4::LookAt(eye, position, {0.0f, 1.0f, 0.0f});
const Mat4 projection = Mat4::Perspective(1.0f, 16.0f / 9.0f, 0.1f, 100.0f);
const Mat4 mvp = projection * view * model;

const Vec3 clip = mvp.TransformPoint3D({0.0f, 0.0f, 0.0f});
```

### Conventions

- **Coordinates:** right-handed, with the camera looking down -z.
- **Order:** matrices apply right to left, so `T * R * S` scales first, then rotates, then translates.
- **Angles:** radians. Positive rotations are counter-clockwise when looking down the axis toward the origin.
- **Clip space:** z in `[0, 1]` and y pointing up.

| Graphics API | What to do |
|---|---|
| Direct3D | Nothing; the conventions match |
| Vulkan | Flip y, by negating `columns[1][1]` of the projection or using a negative viewport height |
| OpenGL | Use `glClipControl` with `GL_ZERO_TO_ONE`, since OpenGL defaults to z in `[-1, 1]` |

### Building matrices

| Function | Description |
|---|---|
| `Identity()` | The identity matrix |
| `Translation(offset)`, `Scaling(scale)` | Move and scale |
| `RotationX(angle)`, `RotationY(angle)`, `RotationZ(angle)` | Rotate around one axis |
| `Rotation(axis, angle)` | Rotate around any non-zero axis, which doesn't need to be unit length |
| `Transform2D(position, rotation, scale)` | Translate, rotate around z, and scale for 2D |
| `Ortho(left, right, bottom, top, nearPlane, farPlane)` | Orthographic projection. The depth range defaults to -1 to 1 |
| `Perspective(fovY, aspect, nearPlane, farPlane)` | Perspective projection. `fovY` is the full vertical angle |
| `LookAt(eye, target, up)` | Camera view matrix |

Inputs aren't validated, so bad values give a degenerate matrix instead of an error. For `Perspective`, `nearPlane` must be above zero and `farPlane` above `nearPlane`. For `LookAt`, `eye` must differ from `target`, and `up` must not be parallel to the view direction.

### Using matrices

| Member | Description |
|---|---|
| `operator*` | Combines two matrices |
| `Inverse()` | The inverse. A singular matrix has none, and the result contains infinities or NaNs |
| `Transpose()` | Swaps rows and columns |
| `TransformPoint(Vec2)` | Transforms as `(x, y, 0, 1)` and returns x and y, without dividing by w |
| `TransformPoint3D(Vec3)` | Transforms as `(x, y, z, 1)` and divides by w, so it works with projections |
| `TransformDirection(Vec3)` | Transforms as `(x, y, z, 0)`, ignoring translation. Not normalized |
| `Data()` | Pointer to the 16 floats |
| `columns[column][row]` | Direct element access |

To transform normals under non-uniform scale, use `matrix.Inverse().Transpose().TransformDirection(normal)`, then normalize the result.

## Vec2 and Vec3

`#include "LiLib/Math/Vec2.h"` and `#include "LiLib/Math/Vec3.h"`

Plain float vectors, passed by value.

```cpp
#include "LiLib/Math/Vec2.h"
#include "LiLib/Math/Vec3.h"

const Vec2 a{3.0f, 4.0f};
const float length = a.Length(); // 5
const Vec2 unit = a.Normalize();
const Vec2 turned = a.Rotate(1.57f);

const Vec3 up = Vec3{1.0f, 0.0f, 0.0f}.Cross({0.0f, 0.0f, -1.0f});
const Vec3 scaled = 2.0f * up + Vec3{1.0f, 1.0f, 1.0f};
```

| Member | Description |
|---|---|
| `+`, `-`, unary `-` | Component-wise add and subtract, and negate |
| `*`, `/` | By another vector (component-wise) or by a float |
| `+=`, `-=`, `*=` | In-place add, subtract, and multiply by a float |
| `==`, `!=` | Exact comparison |
| `Dot(other)` | Dot product |
| `Cross(other)` | `Vec3` returns a vector. `Vec2` returns the z component as a float |
| `Length()`, `LengthSquared()`, `Distance(other)` | Measurements |
| `Normalize()` | Returns a unit vector, or zero if the length is 1e-6 or less |
| `Lerp(to, t)` | Linear interpolation |
| `Rotate(angle)` | `Vec2` only. Counter-clockwise, in radians |

Missing components in braces default to zero, so `Vec3{1.0f, 2.0f}` compiles with `z` at 0. Because `{x, y}` can initialize either type, a `Mat4` method can't overload on both, which is why the 3D point transform is named `TransformPoint3D`.

## Rect

`#include "LiLib/Math/Rect.h"`

An axis-aligned rectangle, given by its minimum corner and size.

```cpp
#include "LiLib/Math/Rect.h"

const Rect a{0.0f, 0.0f, 2.0f, 2.0f};
const Rect b{2.0f, 0.0f, 1.0f, 1.0f};
const bool inside = a.Contains({2.0f, 1.0f}); // true: edges count as inside
const bool overlaps = a.Overlaps(b);          // false: touching isn't overlapping
```

## Color

`#include "LiLib/Math/Color.h"`

A linear RGBA color with float channels. They normally run from 0 to 1, and values above 1 are allowed for HDR.

```cpp
#include "LiLib/Math/Color.h"

const Color orange = Color::FromHex(0xFF8000FF);
const Color blended = orange.Lerp(Colors::White, 0.5f);
const Color tinted = blended * Colors::Red;
const uint32_t packed = tinted.ToHex();
```

| Member | Description |
|---|---|
| `Color::FromHex(rgba)` | Builds a color from `0xRRGGBBAA` |
| `ToHex()` | Packs into `0xRRGGBBAA`, clamping channels to 0 to 1 and rounding |
| `Lerp(to, t)` | Linear interpolation |
| `*`, `==` | Channel-wise multiply, and exact comparison |

`FromHex` and `ToHex` don't convert between sRGB and linear, so hex values from a design tool need gamma conversion to be truly linear.

Constants are in the `Colors` namespace: `White`, `Black`, `Transparent`, `Red`, `Green`, `Blue` and `Yellow`.

## Random

`#include "LiLib/Math/Random.h"`

A deterministic xorshift random number generator, where the same seed always produces the same sequence.

```cpp
#include "LiLib/Math/Random.h"

Random random(42);
const uint64_t bits = random.NextU64();
const float unit = random.NextFloat();          // In [0, 1)
const float value = random.Range(-5.0f, 5.0f);
random.SetSeed(42);
```

A seed of 0 is replaced with 1. It isn't thread-safe, so give each thread its own instance. It isn't suitable for cryptography or security.

## Math utilities

`#include "LiLib/Math/MathUtil.h"`

| Name | Description |
|---|---|
| `Math::Pi`, `Math::TwoPi` | Float constants |
| `Math::DegToRad`, `Math::RadToDeg` | Multiply by these to convert between degrees and radians |
| `Math::Lerp(a, b, t)` | Linear interpolation |
| `Math::Saturate(value)` | Clamps to 0 to 1 |

For other ranges, use `Clamp` from [Core](Core.md#types-and-utilities).
