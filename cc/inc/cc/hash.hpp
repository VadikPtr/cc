#pragma once
#include "cc/common.hpp"

namespace cc {
  // --- generic data hashes

  u64 hash_wy(const void* key, size_t len);
  u32 hash_crc32(const void* data, size_t len);
  u32 hash_fnv32(const void* data, size_t len);
  u64 hash_fnv64(const void* data, size_t len);

  // --- hash

  template <typename T>
  u64 hash(const T& key);

  template <Hashable T>
  u64 hash(const T& key) {
    return key.hash();
  }

  template <PointerType T>
  u64 hash(const T& key) {
    return hash(u64(key));
  }
}  // namespace cc
