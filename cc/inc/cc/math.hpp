#pragma once
#include "cc/common.hpp"
#include "cc/fmt.hpp"

// for faster builds all impl goes to cpp file
#if defined(DEBUG)
  #define mMathInline 0
  #define mMathInlineFunc
#else
  #define mMathInline 1
  #define mMathInlineFunc inline
#endif

inline constexpr f32 g_pi  = 3.141592653589793f;
inline constexpr f32 g_eps = 0.0001f;

// simplified types for glsl
struct Float2 {
  f32 x = 0, y = 0;
};
struct Float3 {
  f32 x = 0, y = 0, z = 0;
};
struct Float4 {
  f32 x = 0, y = 0, z = 0, w = 0;
};
struct UInt2 {
  u32 x = 0, y = 0;
};
struct Int2 {
  s32 x = 0, y = 0;
};
struct USize2 {
  u32 x = 0, y = 0, width = 0, height = 0;
};
struct ISize2 {
  s32 x = 0, y = 0, width = 0, height = 0;
};
struct FSize2 {
  f32 x = 0, y = 0, width = 0, height = 0;
};
struct FSize3 {
  f32 x = 0, y = 0, z = 0, width = 0, height = 0, wide = 0;
};


struct Vec2 {
  f32 x, y;

  Vec2();
  Vec2(f32 v);
  Vec2(Float2 v);
  Vec2(f32 x, f32 y);

  Vec2(const Vec2&)            = default;
  Vec2& operator=(const Vec2&) = default;

  f32& operator[](size_t i);
  f32  operator[](size_t i) const;

  Vec2& operator+=(const Vec2& a);
  Vec2& operator-=(const Vec2& a);

  operator Int2() const;
};

Vec2 operator-(const Vec2& a);
Vec2 operator+(const Vec2& a, const Vec2& b);
Vec2 operator-(const Vec2& a, const Vec2& b);
Vec2 operator*(const Vec2& a, const Vec2& b);
Vec2 operator/(const Vec2& a, const Vec2& b);
Vec2 operator*(const Vec2& a, f32 b);
Vec2 operator/(const Vec2& a, f32 b);
Vec2 operator*(f32 a, const Vec2& b);
Vec2 operator/(f32 a, const Vec2& b);
f32  dot(const Vec2& a, const Vec2& b);


struct Vec3 {
  f32 x, y, z;

  Vec3();
  Vec3(f32 v);
  Vec3(Float3 v);
  Vec3(f32 x, f32 y, f32 z);

  Vec3(const Vec3&)            = default;
  Vec3& operator=(const Vec3&) = default;

  f32&  operator[](size_t i);
  f32   operator[](size_t i) const;
  Vec3& operator+=(const Vec3& a);
  Vec3& operator-=(const Vec3& a);
  Vec3& operator*=(f32 a);

  f32  dot(const Vec3& a) const;
  Vec3 cross(const Vec3& a) const;
  f32  len_sqr() const;
  f32  len() const;
  Vec3 normalized() const;
  Vec3 normalized_or(Vec3 v) const;
  Vec3 rotate_x(f32 angle) const;
  Vec3 rotate_y(f32 angle) const;
  Vec3 rotate_z(f32 angle) const;
  Vec2 to_vec2() const;

  static Vec3 from_color_hex(u32 value);

  static Vec3 up;
  static Vec3 right;
  static Vec3 forward;
};

Vec3 operator-(const Vec3& a);
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& a, const Vec3& b);
Vec3 operator/(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& a, f32 b);
Vec3 operator/(const Vec3& a, f32 b);
Vec3 operator*(f32 a, const Vec3& b);
Vec3 operator/(f32 a, const Vec3& b);
f32  dot(const Vec3& a, const Vec3& b);
Vec3 cross(const Vec3& a, const Vec3& b);


struct Vec4 {
  f32 x, y, z, w;

  Vec4();
  Vec4(f32 v);
  Vec4(const Vec3& v, f32 w);
  Vec4(Float4 v);
  Vec4(f32 x, f32 y, f32 z, f32 w);

  Vec4(const Vec4&)            = default;
  Vec4& operator=(const Vec4&) = default;

  f32& operator[](size_t i);
  f32  operator[](size_t i) const;

  f32  dot(const Vec4& a) const;
  f32  len_sqr() const;
  f32  len() const;
  Vec4 normalized() const;
  Vec3 to_vec3() const;
};

Vec4 operator-(const Vec4& a);
Vec4 operator+(const Vec4& a, const Vec4& b);
Vec4 operator-(const Vec4& a, const Vec4& b);
Vec4 operator*(const Vec4& a, const Vec4& b);
Vec4 operator/(const Vec4& a, const Vec4& b);
Vec4 operator*(const Vec4& a, f32 b);
Vec4 operator/(const Vec4& a, f32 b);
Vec4 operator*(f32 a, const Vec4& b);
Vec4 operator/(f32 a, const Vec4& b);
f32  dot(const Vec4& a, const Vec4& b);


// column major
struct Mat2 {
  using Column = Vec2;
  using Row    = Vec2;
  Column v[2];

  Mat2();       // identity
  Mat2(f32 a);  // diagonal
  Mat2(f32 a, f32 b);
  Mat2(Column a, Column b);

  Mat2(const Mat2&)            = default;
  Mat2& operator=(const Mat2&) = default;

  Column& col(size_t i);
  Column  col(size_t i) const;
  Row     row(size_t i) const;

  f32  determinant() const;
  Mat2 inverse() const;
  Mat2 transpose() const;
};

Mat2 operator*(const Mat2& a, const Mat2& b);
Vec2 operator*(const Mat2& m, const Vec2& v);
Mat2 operator*(const Mat2& m, f32 a);
Mat2 operator*(f32 a, const Mat2& m);
Mat2 operator-(const Mat2& a);


// column major
struct Mat3 {
  using Column = Vec3;
  using Row    = Vec3;
  Column v[3];

  Mat3();       // identity
  Mat3(f32 a);  // diagonal
  Mat3(f32 a, f32 b, f32 c);
  Mat3(Column a, Column b, Column c);

  Mat3(const Mat3&)            = default;
  Mat3& operator=(const Mat3&) = default;

  Column& col(size_t i);
  Column  col(size_t i) const;
  Row     row(size_t i) const;

  f32  determinant() const;
  Mat3 inverse() const;
  Mat3 transpose() const;

  static Mat3 translation(const Vec2& v);
  static Mat3 scaling(const Vec2& v);
};

Mat3 operator*(const Mat3& a, const Mat3& b);
Vec3 operator*(const Mat3& m, const Vec3& v);
Vec2 operator*(const Mat3& m, const Vec2& v);
Mat3 operator*(const Mat3& m, f32 a);
Mat3 operator*(f32 a, const Mat3& m);
Mat3 operator-(const Mat3& a);


// column major
struct Mat4 {
  using Column = Vec4;
  using Row    = Vec4;
  Column v[4];

  Mat4();       // identity
  Mat4(f32 a);  // diagonal
  Mat4(const Mat3& a);
  Mat4(f32 a, f32 b, f32 c, f32 d);
  Mat4(Column a, Column b, Column c, Column d);

  Mat4(const Mat4&)            = default;
  Mat4& operator=(const Mat4&) = default;

  Column& col(size_t i);
  Column  col(size_t i) const;
  Row     row(size_t i) const;

  f32  determinant() const;
  Mat4 inverse() const;
  Mat4 transpose() const;
  Mat3 to_mat3() const;

  static Mat4 translation(const Vec3& v);
  static Mat4 rotation(const Vec3& axis, f32 angle);
  static Mat4 scaling(const Vec3& v);
  static Mat4 look_at(const Vec3& pos, const Vec3& target,
                      const Vec3& up);  // right-handed
  static Mat4 perspective(f32 fovY, f32 aspect, f32 z_near,
                          f32 z_far);  // right-handed, z = -1..1
};

Mat4 operator*(const Mat4& a, const Mat4& b);
Vec4 operator*(const Mat4& m, const Vec4& v);
Mat4 operator*(const Mat4& m, f32 a);
Mat4 operator*(f32 a, const Mat4& m);
Mat4 operator-(const Mat4& a);


struct Quat {
  f32 x, y, z, w;

  Quat();
  Quat(f32 x, f32 y, f32 z, f32 w);

  Quat(const Quat&)            = default;
  Quat& operator=(const Quat&) = default;

  f32  dot(const Quat& a) const;
  Quat cross(const Quat& a) const;
  f32  len() const;
  f32  len_sqr() const;
  Quat normalized() const;
  Quat conjugate() const;
  Quat inverse() const;
  Mat3 to_mat3() const;
  Mat4 to_mat4() const;

  static Quat from_euler(f32 yaw, f32 pitch, f32 roll);  // in radians
  static Quat from_euler(Vec3 angles);                   // in radians
};

Quat operator-(const Quat& a);
Quat operator+(const Quat& a, const Quat& b);
Quat operator-(const Quat& a, const Quat& b);
Quat operator*(const Quat& a, const Quat& b);
Quat operator*(const Quat& a, f32 b);
Quat operator*(f32 a, const Quat& b);
Quat operator/(const Quat& a, f32 b);
f32  dot(const Quat& a, const Quat& b);
Quat cross(const Quat& a, const Quat& b);


bool feq(f32 a, f32 b, f32 eps = g_eps);
bool feq(Vec2 a, Vec2 b, f32 eps = g_eps);
bool feq(Vec3 a, Vec3 b, f32 eps = g_eps);
bool feq(Vec4 a, Vec4 b, f32 eps = g_eps);

f32 ease_in_quad(f32 x);
f32 ease_out_quad(f32 x);
f32 ease_in_out_quad(f32 x);

f32 ease_in_cubic(f32 x);
f32 ease_out_cubic(f32 x);
f32 ease_in_out_cubic(f32 x);

f32  to_radians(f32 x);
Vec3 to_radians(const Vec3& v);
Vec2 lerp(const Vec2& a, const Vec2& b, f32 t);
Vec3 lerp(const Vec3& a, const Vec3& b, f32 t);
Vec4 lerp(const Vec4& a, const Vec4& b, f32 t);

struct CameraMath {
  Vec3 position     = {0, 0, 0};
  Vec3 direction    = {0, 0, 1};  // on 1 meter circle
  f32  focal_length = 35;         // in millimeters
  f32  aspect_ratio = 16.f / 9.f;
  f32  near_plane   = 0.5f;
  f32  far_plane    = 1000.f;

  Mat4 world_to_view() const;
  Mat4 view_to_projection() const;
};

//     _______
//    /     / |
//   |     |  |
// bz|     |  |
//   |     | /by
//   |_____|/
//  c    bx
struct BoundingBoxMath {
  Vec3 center;      // bound box center
  Vec3 bx, by, bz;  // vectors (edges) from 'center', coordinate system of bounding box

  bool is_inside(Vec3 point) const;
  void get_edges(Vec3 (&points)[24]) const;
};

mFmtDeclare(Float2);
mFmtDeclare(Float3);
mFmtDeclare(Float4);
mFmtDeclare(UInt2);
mFmtDeclare(Int2);
mFmtDeclare(USize2);
mFmtDeclare(ISize2);
mFmtDeclare(FSize2);
mFmtDeclare(FSize3);
mFmtDeclare(Vec2);
mFmtDeclare(Vec3);
mFmtDeclare(Vec4);
mFmtDeclare(Quat);
mFmtDeclare(Mat4);
mStrParserDeclare(Vec3);
mStrParserDeclare(Vec4);
mStrParserDeclare(Quat);

#if mMathInline
  #include "cc/math.inl"
#endif
