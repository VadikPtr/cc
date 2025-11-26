#pragma once
#include <type_traits>
#include "cc/math.hpp"

// TODO: operator[] for vectors is done by very strange way so compiler MAYBE will not
//   optimize those code, so it needs reworking
// TODO: for normalization inversesqrt algorithm can help

mMathInlineFunc Vec2::Vec2() : x(0), y(0) {}

mMathInlineFunc Vec2::Vec2(f32 v) : x(v), y(v) {}

mMathInlineFunc Vec2::Vec2(Float2 v) : x(v.x), y(v.y) {}

mMathInlineFunc Vec2::Vec2(f32 x, f32 y) : x(x), y(y) {}

mMathInlineFunc f32& Vec2::operator[](size_t i) {
  return (&x)[i];
}
mMathInlineFunc f32 Vec2::operator[](size_t i) const {
  return (&x)[i];
}
mMathInlineFunc Vec2& Vec2::operator+=(const Vec2& a) {
  x += a.x;
  y += a.y;
  return *this;
}
mMathInlineFunc Vec2& Vec2::operator-=(const Vec2& a) {
  x -= a.x;
  y -= a.y;
  return *this;
}
mMathInlineFunc Vec2::operator Int2() const {
  return {s32(x), s32(y)};
}

mMathInlineFunc Vec2 operator-(const Vec2& a) {
  return {-a.x, -a.y};
}
mMathInlineFunc Vec2 operator+(const Vec2& a, const Vec2& b) {
  return {a.x + b.x, a.y + b.y};
}
mMathInlineFunc Vec2 operator-(const Vec2& a, const Vec2& b) {
  return {a.x - b.x, a.y - b.y};
}
mMathInlineFunc Vec2 operator*(const Vec2& a, const Vec2& b) {
  return {a.x * b.x, a.y * b.y};
}
mMathInlineFunc Vec2 operator/(const Vec2& a, const Vec2& b) {
  return {a.x / b.x, a.y / b.y};
}
mMathInlineFunc Vec2 operator*(const Vec2& a, f32 b) {
  return a * Vec2(b);
}
mMathInlineFunc Vec2 operator/(const Vec2& a, f32 b) {
  return a / Vec2(b);
}
mMathInlineFunc Vec2 operator*(f32 a, const Vec2& b) {
  return Vec2(a) * b;
}
mMathInlineFunc Vec2 operator/(f32 a, const Vec2& b) {
  return Vec2(a) / b;
}
mMathInlineFunc f32 dot(const Vec2& a, const Vec2& b) {
  return a.x * b.x + a.y * b.y;
}

mMathInlineFunc Vec3::Vec3() : x(0), y(0), z(0) {}

mMathInlineFunc Vec3::Vec3(f32 v) : x(v), y(v), z(v) {}

mMathInlineFunc Vec3::Vec3(Float3 v) : x(v.x), y(v.y), z(v.z) {}

mMathInlineFunc Vec3::Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}

mMathInlineFunc f32& Vec3::operator[](size_t i) {
  return (&x)[i];
}
mMathInlineFunc f32 Vec3::operator[](size_t i) const {
  return (&x)[i];
}

mMathInlineFunc Vec3& Vec3::operator+=(const Vec3& a) {
  return *this = *this + a;
}
mMathInlineFunc Vec3& Vec3::operator-=(const Vec3& a) {
  return *this = *this - a;
}
mMathInlineFunc Vec3& Vec3::operator*=(f32 a) {
  return *this = *this * a;
}

mMathInlineFunc f32 Vec3::dot(const Vec3& a) const {
  return x * a.x + y * a.y + z * a.z;
}
mMathInlineFunc Vec3 Vec3::cross(const Vec3& a) const {
  return {y * a.z - a.y * z, z * a.x - a.z * x, x * a.y - a.x * y};
}
mMathInlineFunc f32 Vec3::len_sqr() const {
  return x * x + y * y + z * z;
}
mMathInlineFunc f32 Vec3::len() const {
  return sqrtf(len_sqr());
}
mMathInlineFunc Vec3 Vec3::normalized() const {
  return *this / len();
}
mMathInlineFunc Vec3 Vec3::normalized_or(Vec3 v) const {
  f32 l = len();
  if (l > 0.0001f) {
    return *this / l;
  } else {
    return v;
  }
}

mMathInlineFunc Vec3 operator-(const Vec3& a) {
  return {-a.x, -a.y, -a.z};
}
mMathInlineFunc Vec3 operator+(const Vec3& a, const Vec3& b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}
mMathInlineFunc Vec3 operator-(const Vec3& a, const Vec3& b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}
mMathInlineFunc Vec3 operator*(const Vec3& a, const Vec3& b) {
  return {a.x * b.x, a.y * b.y, a.z * b.z};
}
mMathInlineFunc Vec3 operator/(const Vec3& a, const Vec3& b) {
  return {a.x / b.x, a.y / b.y, a.z / b.z};
}
mMathInlineFunc Vec3 operator*(const Vec3& a, f32 b) {
  return a * Vec3(b);
}
mMathInlineFunc Vec3 operator/(const Vec3& a, f32 b) {
  return a / Vec3(b);
}
mMathInlineFunc Vec3 operator*(f32 a, const Vec3& b) {
  return Vec3(a) * b;
}
mMathInlineFunc Vec3 operator/(f32 a, const Vec3& b) {
  return Vec3(a) / b;
}
mMathInlineFunc f32 dot(const Vec3& a, const Vec3& b) {
  return a.dot(b);
}
mMathInlineFunc Vec3 cross(const Vec3& a, const Vec3& b) {
  return a.cross(b);
}

mMathInlineFunc Vec3 Vec3::rotate_x(f32 angle) const {
  f32 cos = cosf(angle);
  f32 sin = sinf(angle);
  return {x, y * cos - z * sin, y * sin + z * cos};
}

mMathInlineFunc Vec3 Vec3::rotate_y(f32 angle) const {
  f32 cos = cosf(angle);
  f32 sin = sinf(angle);
  return {x * cos + z * sin, y, -x * sin + z * cos};
}

mMathInlineFunc Vec3 Vec3::rotate_z(f32 angle) const {
  f32 cos = cosf(angle);
  f32 sin = sinf(angle);
  return {x * cos - y * sin, x * sin + y * cos, z};
}
mMathInlineFunc Vec2 Vec3::to_vec2() const {
  return {x, y};
}

mMathInlineFunc Vec3 Vec3::from_color_hex(u32 value) {
  return {
      f32(value >> 16 & 0xFF) / 255.0f,  // Extract the RR byte
      f32(value >> 8 & 0xFF) / 255.0f,   // Extract the GG byte
      f32(value & 0xFF) / 255.0f         // Extract the BB byte
  };
}

mMathInlineFunc Vec3 Vec3::up      = {0, 1, 0};   // gl: 0,1,0
mMathInlineFunc Vec3 Vec3::right   = {1, 0, 0};   // gl: 1,0,0
mMathInlineFunc Vec3 Vec3::forward = {0, 0, -1};  // gl: 0,0,-1


mMathInlineFunc Vec4::Vec4() : x(0), y(0), z(0), w(0) {}

mMathInlineFunc Vec4::Vec4(f32 v) : x(v), y(v), z(v), w(v) {}

mMathInlineFunc Vec4::Vec4(const Vec3& v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}

mMathInlineFunc Vec4::Vec4(Float4 v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

mMathInlineFunc Vec4::Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

mMathInlineFunc f32& Vec4::operator[](size_t i) {
  return (&x)[i];
}
mMathInlineFunc f32 Vec4::operator[](size_t i) const {
  return (&x)[i];
}

mMathInlineFunc f32 Vec4::dot(const Vec4& a) const {
  return x * a.x + y * a.y + z * a.z + w * a.w;
}
mMathInlineFunc f32 Vec4::len_sqr() const {
  return dot(*this);
}
mMathInlineFunc f32 Vec4::len() const {
  return sqrtf(len_sqr());
}
mMathInlineFunc Vec4 Vec4::normalized() const {
  return *this / len();
}
mMathInlineFunc Vec3 Vec4::to_vec3() const {
  return {x, y, z};
}

mMathInlineFunc Vec4 operator-(const Vec4& a) {
  return {-a.x, -a.y, -a.z, -a.w};
}
mMathInlineFunc Vec4 operator+(const Vec4& a, const Vec4& b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
mMathInlineFunc Vec4 operator-(const Vec4& a, const Vec4& b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
mMathInlineFunc Vec4 operator*(const Vec4& a, const Vec4& b) {
  return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}
mMathInlineFunc Vec4 operator/(const Vec4& a, const Vec4& b) {
  return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}
mMathInlineFunc Vec4 operator*(const Vec4& a, f32 b) {
  return a * Vec4(b);
}
mMathInlineFunc Vec4 operator/(const Vec4& a, f32 b) {
  return a / Vec4(b);
}
mMathInlineFunc Vec4 operator*(f32 a, const Vec4& b) {
  return Vec4(a) * b;
}
mMathInlineFunc Vec4 operator/(f32 a, const Vec4& b) {
  return Vec4(a) / b;
}
mMathInlineFunc f32 dot(const Vec4& a, const Vec4& b) {
  return a.dot(b);
}

// mat2

mMathInlineFunc Mat2::Mat2(f32 a) : v{Column(a, 0), Column(0, a)} {}

mMathInlineFunc Mat2::Mat2() : Mat2(1) {}

mMathInlineFunc Mat2::Mat2(f32 a, f32 b) : v{Column(a, 0), Column(0, b)} {}

mMathInlineFunc Mat2::Mat2(Column a, Column b) : v{a, b} {}

mMathInlineFunc Mat2::Column& Mat2::col(size_t i) {
  return v[i];
}
mMathInlineFunc Mat2::Column Mat2::col(size_t i) const {
  return v[i];
}
mMathInlineFunc Mat2::Row Mat2::row(size_t i) const {
  return {v[0][i], v[1][i]};
}

mMathInlineFunc f32 Mat2::determinant() const {
  return v[0][0] * v[1][1] - v[0][1] * v[1][0];
}

mMathInlineFunc Mat2 Mat2::inverse() const {
  f32  oneOverDet = 1.0f / determinant();
  Mat2 inverse;
  inverse.v[0][0] = v[1][1] * oneOverDet;
  inverse.v[0][1] = -v[0][1] * oneOverDet;
  inverse.v[1][0] = -v[1][0] * oneOverDet;
  inverse.v[1][1] = v[0][0] * oneOverDet;
  return inverse;
}

mMathInlineFunc Mat2 Mat2::transpose() const {
  return {row(0), row(1)};
}

mMathInlineFunc Mat2 operator*(const Mat2& a, const Mat2& b) {
  auto arow0 = a.row(0);  // rows is bad for processor, cache it first
  auto arow1 = a.row(1);
  return {{dot(arow0, b.col(0)), dot(arow1, b.col(0))},
          {dot(arow0, b.col(1)), dot(arow1, b.col(1))}};
}

mMathInlineFunc Vec2 operator*(const Mat2& m, const Vec2& v) {
  return {dot(m.row(0), v), dot(m.row(1), v)};
}
mMathInlineFunc Mat2 operator*(const Mat2& m, f32 a) {
  return {m.col(0) * a, m.col(1) * a};
}
mMathInlineFunc Mat2 operator*(f32 a, const Mat2& m) {
  return {m.col(0) * a, m.col(1) * a};
}
mMathInlineFunc Mat2 operator-(const Mat2& a) {
  return {-a.col(0), -a.col(1)};
}

// mat3

mMathInlineFunc Mat3::Mat3(f32 a)
    : v{Column(a, 0, 0), Column(0, a, 0), Column(0, 0, a)} {}

mMathInlineFunc Mat3::Mat3() : Mat3(1) {}

mMathInlineFunc Mat3::Mat3(f32 a, f32 b, f32 c)
    : v{Column(a, 0, 0), Column(0, b, 0), Column(0, 0, c)} {}

mMathInlineFunc Mat3::Mat3(Column a, Column b, Column c) : v{a, b, c} {}

mMathInlineFunc Mat3::Column& Mat3::col(size_t i) {
  return v[i];
}
mMathInlineFunc Mat3::Column Mat3::col(size_t i) const {
  return v[i];
}
mMathInlineFunc Mat3::Row Mat3::row(size_t i) const {
  return {v[0][i], v[1][i], v[2][i]};
}

mMathInlineFunc f32 Mat3::determinant() const {
  return v[0][0] * (v[1][1] * v[2][2] - v[2][1] * v[1][2]) -
         v[1][0] * (v[0][1] * v[2][2] - v[2][1] * v[0][2]) +
         v[2][0] * (v[0][1] * v[1][2] - v[1][1] * v[0][2]);
}
mMathInlineFunc Mat3 Mat3::inverse() const {
  f32 oneOverDet = 1.0f / determinant();

  auto c1 = Column(+(v[1][1] * v[2][2] - v[2][1] * v[1][2]) * oneOverDet,
                   -(v[0][1] * v[2][2] - v[2][1] * v[0][2]) * oneOverDet,
                   +(v[0][1] * v[1][2] - v[1][1] * v[0][2]) * oneOverDet);

  auto c2 = Column(-(v[1][0] * v[2][2] - v[2][0] * v[1][2]) * oneOverDet,
                   +(v[0][0] * v[2][2] - v[2][0] * v[0][2]) * oneOverDet,
                   -(v[0][0] * v[1][2] - v[1][0] * v[0][2]) * oneOverDet);

  auto c3 = Column(+(v[1][0] * v[2][1] - v[2][0] * v[1][1]) * oneOverDet,
                   -(v[0][0] * v[2][1] - v[2][0] * v[0][1]) * oneOverDet,
                   +(v[0][0] * v[1][1] - v[1][0] * v[0][1]) * oneOverDet);

  return {c1, c2, c3};
}
mMathInlineFunc Mat3 Mat3::transpose() const {
  return {row(0), row(1), row(2)};
}
mMathInlineFunc Mat3 Mat3::translation(const Vec2& v) {
  return Mat3(Column(1, 0, 0),  //
              Column(0, 1, 0),  //
              Column(v.x, v.y, 1));
}
mMathInlineFunc Mat3 Mat3::scaling(const Vec2& v) {
  return Mat3(Column(v.x, 0, 0),  //
              Column(0, v.y, 0),  //
              Column(0, 0, 1));
}

mMathInlineFunc Mat3 operator*(const Mat3& a, const Mat3& b) {
  auto arow0 = a.row(0);  // rows is bad for processor, cache it first
  auto arow1 = a.row(1);
  auto arow2 = a.row(2);
  return {{dot(arow0, b.col(0)), dot(arow1, b.col(0)), dot(arow2, b.col(0))},
          {dot(arow0, b.col(1)), dot(arow1, b.col(1)), dot(arow2, b.col(1))},
          {dot(arow0, b.col(2)), dot(arow1, b.col(2)), dot(arow2, b.col(2))}};
}

mMathInlineFunc Vec3 operator*(const Mat3& m, const Vec3& v) {
  return {dot(m.row(0), v), dot(m.row(1), v), dot(m.row(2), v)};
}
mMathInlineFunc Vec2 operator*(const Mat3& m, const Vec2& v) {
  Vec3 pr = m * Vec3(v.x, v.y, 1);
  return pr.to_vec2() / pr.z;
}
mMathInlineFunc Mat3 operator*(const Mat3& m, f32 a) {
  return {m.col(0) * a, m.col(1) * a, m.col(2) * a};
}
mMathInlineFunc Mat3 operator*(f32 a, const Mat3& m) {
  return {m.col(0) * a, m.col(1) * a, m.col(2) * a};
}
mMathInlineFunc Mat3 operator-(const Mat3& a) {
  return {-a.col(0), -a.col(1), -a.col(2)};
}

// mat4

mMathInlineFunc Mat4::Mat4() : Mat4(1) {}

mMathInlineFunc Mat4::Mat4(f32 a)
    : v{Column(a, 0, 0, 0), Column(0, a, 0, 0), Column(0, 0, a, 0), Column(0, 0, 0, a)} {}

mMathInlineFunc Mat4::Mat4(const Mat3& a)
    : v{Column(a.col(0), 0),  //
        Column(a.col(1), 0),  //
        Column(a.col(2), 0),  //
        Column(0, 0, 0, 1)} {}

mMathInlineFunc Mat4::Mat4(f32 a, f32 b, f32 c, f32 d)
    : v{Column(a, 0, 0, 0),  //
        Column(0, b, 0, 0),  //
        Column(0, 0, c, 0),  //
        Column(0, 0, 0, d)} {}

mMathInlineFunc Mat4::Mat4(Column a, Column b, Column c, Column d) : v{a, b, c, d} {}

mMathInlineFunc Mat4::Column& Mat4::col(size_t i) {
  return v[i];
}
mMathInlineFunc Mat4::Column Mat4::col(size_t i) const {
  return v[i];
}
mMathInlineFunc Mat4::Row Mat4::row(size_t i) const {
  return {v[0][i], v[1][i], v[2][i], v[3][i]};
}

mMathInlineFunc f32 Mat4::determinant() const {
  f32  f0 = v[2][2] * v[3][3] - v[3][2] * v[2][3];
  f32  f1 = v[2][1] * v[3][3] - v[3][1] * v[2][3];
  f32  f2 = v[2][1] * v[3][2] - v[3][1] * v[2][2];
  f32  f3 = v[2][0] * v[3][3] - v[3][0] * v[2][3];
  f32  f4 = v[2][0] * v[3][2] - v[3][0] * v[2][2];
  f32  f5 = v[2][0] * v[3][1] - v[3][0] * v[2][1];
  auto d  = Vec4(+(v[1][1] * f0 - v[1][2] * f1 + v[1][3] * f2),
                 -(v[1][0] * f0 - v[1][2] * f3 + v[1][3] * f4),
                 +(v[1][0] * f1 - v[1][1] * f3 + v[1][3] * f5),
                 -(v[1][0] * f2 - v[1][1] * f4 + v[1][2] * f5));
  return v[0][0] * d[0] + v[0][1] * d[1] + v[0][2] * d[2] + v[0][3] * d[3];
}

mMathInlineFunc Mat4 Mat4::inverse() const {
  f32  c00          = v[2][2] * v[3][3] - v[3][2] * v[2][3];
  f32  c02          = v[1][2] * v[3][3] - v[3][2] * v[1][3];
  f32  c03          = v[1][2] * v[2][3] - v[2][2] * v[1][3];
  f32  c04          = v[2][1] * v[3][3] - v[3][1] * v[2][3];
  f32  c06          = v[1][1] * v[3][3] - v[3][1] * v[1][3];
  f32  c07          = v[1][1] * v[2][3] - v[2][1] * v[1][3];
  f32  c08          = v[2][1] * v[3][2] - v[3][1] * v[2][2];
  f32  c10          = v[1][1] * v[3][2] - v[3][1] * v[1][2];
  f32  c11          = v[1][1] * v[2][2] - v[2][1] * v[1][2];
  f32  c12          = v[2][0] * v[3][3] - v[3][0] * v[2][3];
  f32  c14          = v[1][0] * v[3][3] - v[3][0] * v[1][3];
  f32  c15          = v[1][0] * v[2][3] - v[2][0] * v[1][3];
  f32  c16          = v[2][0] * v[3][2] - v[3][0] * v[2][2];
  f32  c18          = v[1][0] * v[3][2] - v[3][0] * v[1][2];
  f32  c19          = v[1][0] * v[2][2] - v[2][0] * v[1][2];
  f32  c20          = v[2][0] * v[3][1] - v[3][0] * v[2][1];
  f32  c22          = v[1][0] * v[3][1] - v[3][0] * v[1][1];
  f32  c23          = v[1][0] * v[2][1] - v[2][0] * v[1][1];
  auto f0           = Vec4(c00, c00, c02, c03);
  auto f1           = Vec4(c04, c04, c06, c07);
  auto f2           = Vec4(c08, c08, c10, c11);
  auto f3           = Vec4(c12, c12, c14, c15);
  auto f4           = Vec4(c16, c16, c18, c19);
  auto f5           = Vec4(c20, c20, c22, c23);
  auto v0           = Vec4(v[1][0], v[0][0], v[0][0], v[0][0]);
  auto v1           = Vec4(v[1][1], v[0][1], v[0][1], v[0][1]);
  auto v2           = Vec4(v[1][2], v[0][2], v[0][2], v[0][2]);
  auto v3           = Vec4(v[1][3], v[0][3], v[0][3], v[0][3]);
  auto i0           = Vec4(v1 * f0 - v2 * f1 + v3 * f2);
  auto i1           = Vec4(v0 * f0 - v2 * f3 + v3 * f4);
  auto i2           = Vec4(v0 * f1 - v1 * f3 + v3 * f5);
  auto i3           = Vec4(v0 * f2 - v1 * f4 + v2 * f5);
  auto sA           = Vec4(+1, -1, +1, -1);
  auto sB           = Vec4(-1, +1, -1, +1);
  auto iv           = Mat4(i0 * sA, i1 * sB, i2 * sA, i3 * sB);
  auto r0           = Vec4(iv.col(0)[0], iv.col(1)[0], iv.col(2)[0], iv.col(3)[0]);
  auto d0           = v[0] * r0;
  f32  det          = (d0.x + d0.y) + (d0.z + d0.w);
  f32  one_over_det = 1.0f / det;
  return iv * one_over_det;
}

mMathInlineFunc Mat4 Mat4::transpose() const {
  return {row(0), row(1), row(2), row(3)};
}

mMathInlineFunc Mat3 Mat4::to_mat3() const {
  return {col(0).to_vec3(), col(1).to_vec3(), col(2).to_vec3()};
}

mMathInlineFunc Mat4 Mat4::translation(const Vec3& v) {
  return {{1, 0, 0, 0},  //
          {0, 1, 0, 0},  //
          {0, 0, 1, 0},  //
          {v.x, v.y, v.z, 1}};
}

mMathInlineFunc Mat4 Mat4::rotation(const Vec3& axis, f32 angle) {
  f32 cos = cosf(angle);
  f32 sin = sinf(angle);

  auto axisN = axis.normalized();
  auto temp  = axisN * (1.0f - cos);

  Vec4 c1 = {cos + temp.x * axisN.x, temp.x * axisN.y + sin * axisN.z,
             temp.x * axisN.z - sin * axisN.y, 0};

  Vec4 c2 = {temp.y * axisN.x - sin * axisN.z, cos + temp.y * axisN.y,
             temp.y * axisN.z + sin * axisN.x, 0};

  Vec4 c3 = {temp.z * axisN.x + sin * axisN.y, temp.z * axisN.y - sin * axisN.x,
             cos + temp.z * axisN.z, 0};

  Vec4 c4 = {0, 0, 0, 1};

  return {c1, c2, c3, c4};
}

mMathInlineFunc Mat4 Mat4::scaling(const Vec3& v) {
  return {{v.x, 0, 0, 0},  //
          {0, v.y, 0, 0},  //
          {0, 0, v.z, 0},  //
          {0, 0, 0, 1}};
}

mMathInlineFunc Mat4 Mat4::look_at(const Vec3& pos, const Vec3& target, const Vec3& up) {
  Vec3 target_to_position = {pos.x - target.x, pos.y - target.y, pos.z - target.z};
  Vec3 a                  = target_to_position.normalized();
  Vec3 b                  = cross(up, a).normalized();
  Vec3 c                  = cross(a, b);
  return {
      {b.x, c.x, a.x, 0},
      {b.y, c.y, a.y, 0},
      {b.z, c.z, a.z, 0},
      {-dot(b, pos), -dot(c, pos), -dot(a, pos), 1},
  };
}

mMathInlineFunc Mat4 Mat4::perspective(f32 fovY, f32 aspect, f32 z_near, f32 z_far) {
  float t = 1.0f / tanf(fovY * 0.5f);
  return {
      {t / aspect, 0, 0, 0},
      {0, t, 0, 0},
      {0, 0, z_far / (z_near - z_far), -1},
      {0, 0, (z_near * z_far) / (z_near - z_far), 0},
  };
}

mMathInlineFunc Mat4 operator*(const Mat4& a, const Mat4& b) {
  auto arow0 = a.row(0);  // rows is bad for processor, cache it first
  auto arow1 = a.row(1);
  auto arow2 = a.row(2);
  auto arow3 = a.row(3);
  return {{dot(arow0, b.col(0)), dot(arow1, b.col(0)), dot(arow2, b.col(0)),
           dot(arow3, b.col(0))},
          {dot(arow0, b.col(1)), dot(arow1, b.col(1)), dot(arow2, b.col(1)),
           dot(arow3, b.col(1))},
          {dot(arow0, b.col(2)), dot(arow1, b.col(2)), dot(arow2, b.col(2)),
           dot(arow3, b.col(2))},
          {dot(arow0, b.col(3)), dot(arow1, b.col(3)), dot(arow2, b.col(3)),
           dot(arow3, b.col(3))}};
}

mMathInlineFunc Vec4 operator*(const Mat4& m, const Vec4& v) {
  // return {dot(m.row(0), v), dot(m.row(1), v), dot(m.row(2), v), dot(m.row(3), v)};
  //  actually better for vectorization??
  auto mv0 = Vec4(v[0]);
  auto mv1 = Vec4(v[1]);
  Vec4 ml0 = m.col(0) * mv0;
  Vec4 ml1 = m.col(1) * mv1;
  Vec4 a0  = ml0 + ml1;
  auto mv2 = Vec4(v[2]);
  auto mv3 = Vec4(v[3]);
  Vec4 ml2 = m.col(2) * mv2;
  Vec4 ml3 = m.col(3) * mv3;
  Vec4 a1  = ml2 + ml3;
  Vec4 a2  = a0 + a1;
  return a2;
}

mMathInlineFunc Mat4 operator*(const Mat4& m, f32 a) {
  return {m.col(0) * a, m.col(1) * a, m.col(2) * a, m.col(3) * a};
}
mMathInlineFunc Mat4 operator*(f32 a, const Mat4& m) {
  return {m.col(0) * a, m.col(1) * a, m.col(2) * a, m.col(3) * a};
}
mMathInlineFunc Mat4 operator-(const Mat4& a) {
  return {-a.col(0), -a.col(1), -a.col(2), -a.col(3)};
}


mMathInlineFunc Quat::Quat() : x(0), y(0), z(0), w(1) {}

mMathInlineFunc Quat::Quat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

mMathInlineFunc f32 Quat::dot(const Quat& a) const {
  return x * a.x + y * a.y + z * a.z + w * a.w;
}

mMathInlineFunc Quat Quat::cross(const Quat& a) const {
  return {w * a.x + x * a.w + y * a.z - z * a.y, w * a.y + y * a.w + z * a.x - x * a.z,
          w * a.z + z * a.w + x * a.y - y * a.x, w * a.w - x * a.x - y * a.y - z * a.z};
}

mMathInlineFunc f32 Quat::len() const {
  return sqrtf(len_sqr());
}
mMathInlineFunc f32 Quat::len_sqr() const {
  return dot(*this);
}
mMathInlineFunc Quat Quat::normalized() const {
  return *this / len();
}
mMathInlineFunc Quat Quat::conjugate() const {
  return {-x, -y, -z, w};
}
mMathInlineFunc Quat Quat::inverse() const {
  return conjugate() / len_sqr();
}

mMathInlineFunc Mat3 Quat::to_mat3() const {
  f32 xx = x * x;
  f32 yy = y * y;
  f32 zz = z * z;
  f32 xz = x * z;
  f32 xy = x * y;
  f32 yz = y * z;
  f32 wx = w * x;
  f32 wy = w * y;
  f32 wz = w * z;
  return {{1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy)},
          {2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx)},
          {2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy)}};
}

mMathInlineFunc Mat4 Quat::to_mat4() const {
  return Mat4(to_mat3());
}

mMathInlineFunc Quat Quat::from_euler(f32 yaw, f32 pitch, f32 roll) {
  f32 cr = cosf(roll * .5f);
  f32 sr = sinf(roll * .5f);
  f32 cp = cosf(pitch * .5f);
  f32 sp = sinf(pitch * .5f);
  f32 cy = cosf(yaw * .5f);
  f32 sy = sinf(yaw * .5f);
  return {sr * cp * cy - cr * sp * sy, cr * sp * cy + sr * cp * sy,
          cr * cp * sy - sr * sp * cy, cr * cp * cy + sr * sp * sy};
}

mMathInlineFunc Quat Quat::from_euler(Vec3 angles) {
  return from_euler(angles.x, angles.y, angles.z);
}

mMathInlineFunc Quat operator-(const Quat& a) {
  return {-a.x, -a.y, -a.z, -a.w};
}
mMathInlineFunc Quat operator+(const Quat& a, const Quat& b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
mMathInlineFunc Quat operator-(const Quat& a, const Quat& b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

mMathInlineFunc Quat operator*(const Quat& a, const Quat& b) {
  return {a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
          a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
          a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
          a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
}

mMathInlineFunc Quat operator*(const Quat& a, f32 b) {
  return {a.x * b, a.y * b, a.z * b, a.w * b};
}
mMathInlineFunc Quat operator*(f32 a, const Quat& b) {
  return b * a;
}
mMathInlineFunc Quat operator/(const Quat& a, f32 b) {
  return {a.x / b, a.y / b, a.z / b, a.w / b};
}
mMathInlineFunc f32 dot(const Quat& a, const Quat& b) {
  return a.dot(b);
}
mMathInlineFunc Quat cross(const Quat& a, const Quat& b) {
  return a.cross(b);
}

mMathInlineFunc bool feq(f32 a, f32 b, f32 eps) {
  return std::abs(a - b) < eps;
}
mMathInlineFunc bool feq(Vec2 a, Vec2 b, f32 eps) {
  return feq(a.x, b.x, eps) and feq(a.y, b.y, eps);
}
mMathInlineFunc bool feq(Vec3 a, Vec3 b, f32 eps) {
  return feq(a.x, b.x, eps) and feq(a.y, b.y, eps) and feq(a.z, b.z, eps);
}
mMathInlineFunc bool feq(Vec4 a, Vec4 b, f32 eps) {
  return feq(a.x, b.x, eps) and feq(a.y, b.y, eps) and feq(a.z, b.z, eps) and
         feq(a.w, b.w, eps);
}

mMathInlineFunc f32 ease_in_quad(f32 x) {
  return x * x;
}
mMathInlineFunc f32 ease_out_quad(f32 x) {
  return -(x * (x - 2));
}
mMathInlineFunc f32 ease_in_out_quad(f32 x) {
  return x < 0.5f ? 2 * x * x : (-2 * x * x) + (4 * x) - 1;
}

mMathInlineFunc f32 ease_in_cubic(f32 x) {
  return x * x * x;
}
mMathInlineFunc f32 ease_out_cubic(f32 x) {
  f32 f = x - 1;
  return f * f * f + 1;
}
mMathInlineFunc f32 ease_in_out_cubic(f32 x) {
  if (x < 0.5f) {
    return 4 * x * x * x;
  }
  f32 f = ((2 * x) - 2);
  return 0.5f * f * f * f + 1;
}

mMathInlineFunc f32 to_radians(f32 x) {
  return x / 180.0f * g_pi;
}
mMathInlineFunc Vec3 to_radians(const Vec3& v) {
  return {to_radians(v.x), to_radians(v.y), to_radians(v.z)};
}
mMathInlineFunc Vec2 lerp(const Vec2& a, const Vec2& b, f32 t) {
  return a * (1.0f - t) + b * t;
}
mMathInlineFunc Vec3 lerp(const Vec3& a, const Vec3& b, f32 t) {
  return a * (1.0f - t) + b * t;
}
mMathInlineFunc Vec4 lerp(const Vec4& a, const Vec4& b, f32 t) {
  return a * (1.0f - t) + b * t;
}

mMathInlineFunc mFmtImpl(Float2) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(Float3) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(Float4) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(UInt2) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(Int2) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(USize2) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(ISize2) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(FSize2) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}
mMathInlineFunc mFmtImpl(FSize3) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}

mMathInlineFunc mFmtImpl(Vec2) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}

mMathInlineFunc mFmtImpl(Vec3) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}

mMathInlineFunc mFmtImpl(Vec4) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}

mMathInlineFunc mFmtImpl(Quat) {
  using TVec = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
  using Type = decltype(v.x);
  ArrView arr{(Type*)&v, sizeof(TVec) / sizeof(Type)};
  Fmt<ArrView<Type>>::format(arr, out);
}

mMathInlineFunc mFmtImpl(Mat4) {
  out.appendf(
      "<%10.4f %10.4f %10.4f %10.4f\n"
      " %10.4f %10.4f %10.4f %10.4f\n"
      " %10.4f %10.4f %10.4f %10.4f\n"
      " %10.4f %10.4f %10.4f %10.4f>\n",
      (f64)v.col(0)[0], (f64)v.col(1)[0], (f64)v.col(2)[0], (f64)v.col(3)[0],  //
      (f64)v.col(0)[1], (f64)v.col(1)[1], (f64)v.col(2)[1], (f64)v.col(3)[1],  //
      (f64)v.col(0)[2], (f64)v.col(1)[2], (f64)v.col(2)[2], (f64)v.col(3)[2],  //
      (f64)v.col(0)[3], (f64)v.col(1)[3], (f64)v.col(2)[3], (f64)v.col(3)[3]);
}

mMathInlineFunc mStrParserImpl(Vec3) {
  ArrView arr{reinterpret_cast<f32*>(&out), 3};
  return StrParser<ArrView<f32>>::try_parse(str, arr);
}

mMathInlineFunc mStrParserImpl(Vec4) {
  ArrView arr{reinterpret_cast<f32*>(&out), 4};
  return StrParser<ArrView<f32>>::try_parse(str, arr);
}

mMathInlineFunc mStrParserImpl(Quat) {
  ArrView arr{reinterpret_cast<f32*>(&out), 4};
  return StrParser<ArrView<f32>>::try_parse(str, arr);
}
