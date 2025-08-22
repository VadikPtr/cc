#include "cc/math.hpp"

#include "cc/str-view.hpp"

#if !mMathInline
  #include "cc/math.inl"
#endif

namespace {
  f32 camera_mm_to_fovy(f32 mm) {
    f32 focal_length  = mm * 0.001f;     // in meters
    f32 sensor_length = 23.9f * 0.001f;  // in meters
    return 2.0f * atanf(sensor_length / (2.0f * focal_length));
  }
}  // namespace

Mat4 CameraMath::world_to_view() const {
  return Mat4::look_at(position, position + direction, Vec3::up);
}

Mat4 CameraMath::view_to_projection() const {
  // todo: this needs to be precalculated, not changed every frame
  f32 fov_y = camera_mm_to_fovy(focal_length);
  return Mat4::perspective(fov_y, aspect_ratio, near_plane, far_plane);
}

bool BoundingBoxMath::is_inside(Vec3 point) const {
  Vec3 p = point - center;  // relative point coordinates in bound box coordinates
  auto is_inside_projection = [p](Vec3 axis) {
    auto proj_p_to_axis = p.dot(axis) / axis.len_sqr();  // normalized
    return 0.0f <= proj_p_to_axis && proj_p_to_axis <= 1.0f;
  };
  return is_inside_projection(bx) && is_inside_projection(by) && is_inside_projection(bz);
}

void BoundingBoxMath::get_edges(Vec3 (&points)[24]) const {
  auto A1 = center;
  auto B1 = center + bx;
  auto C1 = center + bx + by;
  auto D1 = center + by;

  auto A2 = A1 + bz;
  auto B2 = B1 + bz;
  auto C2 = C1 + bz;
  auto D2 = D1 + bz;

  Vec3 connections[] = {
      // clang-format off
      A1,B1,  B1,C1,  C1,D1,  D1,A1,  // 1 floor of box
      A2,B2,  B2,C2,  C2,D2,  D2,A2,  // 2 floor of box
      A1,A2,  B1,B2,  C1,C2,  D1,D2,  // walls of box
      // clang-format on
  };
  copy(ArrView(connections), ArrView(points));
}
