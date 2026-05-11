#pragma once
#include "cc/common.hpp"

namespace cc {
  // --- generic data hashes

  u64 hash_wy(const void* key, size_t len);
  u32 hash_crc32(const void* data, size_t len);
  u32 hash_fnv32(const void* data, size_t len);
  u32 hash_combine_fnv32(u32 hash1, u32 hash2);
  u32 hash_combine_fnv32(u32 hash1, u32 hash2, u32 hash3);
  u32 hash_combine_fnv32(u32 hash1, u32 hash2, u32 hash3, u32 hash4);
  u32 hash_combine_fnv32(u32 hash1, u32 hash2, u32 hash3, u32 hash4, u32 hash5);

  constexpr u64 hash_fnv64(const void* data, size_t len) {
    auto bp   = (unsigned char*)data;
    u64  hval = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; ++i) {
      hval *= 0x100000001b3ULL;
      hval ^= (u64)bp[i];
    }
    return hval;
  }

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
