#include "cc/common.hpp"

bool ptr_intersects(const void* a, size_t a_size, const void* b, size_t b_size) {
  if (!a || !b) {  // nullptr cannot intersect
    return false;
  }
  auto      start_a = (uintptr_t)a;
  uintptr_t end_a   = start_a + a_size;
  auto      start_b = (uintptr_t)b;
  uintptr_t end_b   = start_b + b_size;
  return start_a < end_b && start_b < end_a;
}
