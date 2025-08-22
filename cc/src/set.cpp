#include "cc/set.hpp"

#include "cc/dict.hpp"

// verstable adaptation for c++.
// see LICENSE.verstable for more information.

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
  #include <intrin.h>
  #pragma intrinsic(_BitScanForward64)
  #pragma intrinsic(_BitScanReverse64)
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wcast-align"

using namespace details;

namespace {
  constexpr u16    g_vt_empty                       = 0x0000;
  constexpr u16    g_vt_hash_frag_mask              = 0xF000;
  constexpr u16    g_vt_in_home_bucket_mask         = 0x0800;
  constexpr u16    g_vt_displacement_mask           = 0x07FF;
  constexpr u16    g_vt_empty_placeholder_metadatum = g_vt_empty;
  constexpr size_t g_vt_min_nonzero_bucket_count    = 8;
  constexpr f64    g_vt_max_load                    = 0.9;

  u16 vt_hashfrag(u64 hash) {
    return (hash >> 48) & g_vt_hash_frag_mask;
  }

  size_t vt_quadratic(u16 displacement) {
    return ((size_t)displacement * displacement + displacement) / 2;
  }

  size_t vt_min_bucket_count_for_size(size_t size) {
    if (size == 0) {
      return 0;
    }
    // Round up to a power of two.
    size_t bucket_count = g_vt_min_nonzero_bucket_count;
    while (size > bucket_count * g_vt_max_load) {
      bucket_count *= 2;
    }
    return bucket_count;
  }

#if defined(__GNUC__) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
  int vt_first_nonzero_uint16(u64 val) {
    const u16 endian_checker = 0x0001;
    // Little-endian (the compiler will optimize away
    // the check at -O1 and above).
    if (*(const char*)&endian_checker) {
      return __builtin_ctzll(val) / 16;
    }
    return __builtin_clzll(val) / 16;
  }
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
  int vt_first_nonzero_uint16(u64 val) {
    unsigned long result;

    const u16 endian_checker = 0x0001;
    if (*(const char*)&endian_checker) {
      _BitScanForward64(&result, val);
    } else {
      _BitScanReverse64(&result, val);
      result = 63 - result;
    }

    return result / 16;
  }

#else
  int vt_first_nonzero_uint16(u64 val) {
    int result = 0;

    uint32_t half;
    memcpy(&half, &val, sizeof(uint32_t));
    if (!half) result += 2;

    u16 quarter;
    memcpy(&quarter, (char*)&val + result * sizeof(u16), sizeof(u16));
    if (!quarter) result += 1;

    return result;
  }
#endif
}  // namespace


void SetVItr::fast_forward() {
  while (true) {
    u64 metadata2;
    memcpy(&metadata2, metadatum_, sizeof(u64));
    if (metadata2) {
      int offset = vt_first_nonzero_uint16(metadata2);
      data_      = static_cast<u8*>(data_) + offset * vtable_->bucket_size;
      metadatum_ += offset;
      home_bucket_ = SIZE_MAX;
      return;
    }
    data_ = static_cast<u8*>(data_) + 4 * vtable_->bucket_size;
    metadatum_ += 4;
  }
}

SetVItr& SetVItr::operator++() {
  data_ = static_cast<u8*>(data_) + vtable_->bucket_size;
  ++metadatum_;
  fast_forward();
  return *this;
}

bool SetV::init(const SetV& other) {
  init(other.vtable_);
  key_count_    = other.key_count_;
  buckets_mask_ = other.buckets_mask_;

  if (!buckets_mask_) {
    return true;
  }

  void* allocation = malloc(total_alloc_size());
  if (!allocation) {
    return false;
  }

  buckets_  = allocation;
  metadata_ = (u16*)((u8*)allocation + metadata_offset());
  for (size_t i = 0; i < bucket_count(); ++i) {
    metadata_[i] = other.metadata_[i];
    if (metadata_[i] != g_vt_empty) {
      vtable_->copy_key(get_bucket(i), other.get_bucket(i));
    }
  }
  return true;
}

void SetV::init(SetV&& other) {
  swap(key_count_, other.key_count_);
  swap(buckets_mask_, other.buckets_mask_);
  swap(buckets_, other.buckets_);
  swap(metadata_, other.metadata_);
  vtable_ = other.vtable_;
}

void SetV::init(const SetVTable* a_vtable) {
  key_count_    = 0;
  buckets_mask_ = 0;
  buckets_      = nullptr;
  metadata_     = (u16*)&g_vt_empty_placeholder_metadatum;
  vtable_       = a_vtable;
}

void SetV::clear() {
  if (!key_count_) {
    return;
  }
  for (size_t i = 0; i < bucket_count(); ++i) {
    if (metadata_[i] != g_vt_empty) {
      vtable_->destroy_key(get_bucket(i));
    }
    metadata_[i] = g_vt_empty;
  }
  key_count_ = 0;
}

void SetV::destroy() {
  if (!buckets_mask_) {
    return;
  }
  clear();
  free(buckets_);
  init(vtable_);
}

SetVItr SetV::insert(void* key) {
  while (true) {
    SetVItr itr = insert_raw(key, false, true);
    if (!itr.is_end() ||
        !rehash(buckets_mask_ ? bucket_count() * 2 : g_vt_min_nonzero_bucket_count)) {
      return itr;
    }
  }
}

SetVItr SetV::get(void* key) {
  u64    hash        = vtable_->hash(key);
  size_t home_bucket = hash & buckets_mask_;

  if (!(metadata_[home_bucket] & g_vt_in_home_bucket_mask)) {
    return end();
  }

  u16    hashfrag = vt_hashfrag(hash);
  size_t bucket   = home_bucket;

  while (true) {
    if ((metadata_[bucket] & g_vt_hash_frag_mask) == hashfrag &&
        vtable_->equals_key(get_bucket(bucket), key)) {
      SetVItr itr;
      itr.data_         = get_bucket(bucket);
      itr.metadatum_    = metadata_ + bucket;
      itr.metadata_end_ = metadata_ + buckets_mask_ + 1;
      itr.home_bucket_  = home_bucket;
      itr.vtable_       = vtable_;
      return itr;
    }

    u16 displacement = metadata_[bucket] & g_vt_displacement_mask;
    if (displacement == g_vt_displacement_mask) {
      return end();
    }

    bucket = (home_bucket + vt_quadratic(displacement)) & buckets_mask_;
  }
}

bool SetV::erase_itr_raw(SetVItr itr) {
  --key_count_;
  size_t itr_bucket = itr.metadatum_ - metadata_;

  if (metadata_[itr_bucket] & g_vt_in_home_bucket_mask &&
      (metadata_[itr_bucket] & g_vt_displacement_mask) == g_vt_displacement_mask) {
    vtable_->destroy_key(get_bucket(itr_bucket));
    metadata_[itr_bucket] = g_vt_empty;
    return true;
  }

  if (itr.home_bucket_ == SIZE_MAX) {
    if (metadata_[itr_bucket] & g_vt_in_home_bucket_mask) {
      itr.home_bucket_ = itr_bucket;
    } else {
      itr.home_bucket_ = vtable_->hash(get_bucket(itr_bucket)) & buckets_mask_;
    }
  }

  vtable_->destroy_key(get_bucket(itr_bucket));

  if ((metadata_[itr_bucket] & g_vt_displacement_mask) == g_vt_displacement_mask) {
    size_t bucket = itr.home_bucket_;
    while (true) {
      u16    displacement = metadata_[bucket] & g_vt_displacement_mask;
      size_t next = (itr.home_bucket_ + vt_quadratic(displacement)) & buckets_mask_;
      if (next == itr_bucket) {
        metadata_[bucket] |= g_vt_displacement_mask;
        metadata_[itr_bucket] = g_vt_empty;
        return true;
      }
      bucket = next;
    }
  }

  size_t bucket = itr_bucket;
  while (true) {
    size_t prev = bucket;
    bucket =
        (itr.home_bucket_ + vt_quadratic(metadata_[bucket] & g_vt_displacement_mask)) &
        buckets_mask_;

    if ((metadata_[bucket] & g_vt_displacement_mask) == g_vt_displacement_mask) {
      vtable_->move_key(get_bucket(itr_bucket), get_bucket(bucket));
      metadata_[itr_bucket] = (metadata_[itr_bucket] & ~g_vt_hash_frag_mask) |
                              (metadata_[bucket] & g_vt_hash_frag_mask);
      metadata_[prev] |= g_vt_displacement_mask;
      metadata_[bucket] = g_vt_empty;

      if (bucket > itr_bucket) {
        return false;
      }
      return true;
    }
  }
}

bool SetV::erase(void* key) {
  SetVItr itr = get(key);
  if (itr.is_end()) {
    return false;
  }
  erase_itr_raw(itr);
  return true;
}

SetVItr SetV::begin() const {
  if (!key_count_) {
    return end();
  }
  SetVItr itr;
  itr.data_         = buckets_;
  itr.metadatum_    = metadata_;
  itr.metadata_end_ = metadata_ + buckets_mask_ + 1;
  itr.home_bucket_  = SIZE_MAX;
  itr.vtable_       = vtable_;
  itr.fast_forward();
  return itr;
}

size_t SetV::total_alloc_size() const {
  return metadata_offset() + (buckets_mask_ + 1 + 4) * sizeof(u16);
}

size_t SetV::size() const {
  return key_count_;
}

size_t SetV::bucket_count() const {
  return buckets_mask_ + (bool)buckets_mask_;
}

size_t SetV::metadata_offset() const {
  return (((buckets_mask_ + 1) * vtable_->bucket_size + sizeof(u16) - 1) / sizeof(u16)) *
         sizeof(u16);
}

u8* SetV::get_bucket(size_t idx) const {
  return static_cast<u8*>(buckets_) + idx * vtable_->bucket_size;
}

bool SetV::find_first_empty(size_t home_bucket, size_t& empty, u16& displacement) const {
  displacement               = 1;
  size_t linear_displacement = 1;
  while (true) {
    empty = (home_bucket + linear_displacement) & buckets_mask_;
    if (metadata_[empty] == g_vt_empty) {
      return true;
    }
    if (++displacement == g_vt_displacement_mask) {
      return false;
    }
    linear_displacement += displacement;
  }
}

size_t SetV::find_insert_location_in_chain(size_t home_bucket,
                                           u16    displacement_to_empty) const {
  size_t candidate = home_bucket;
  while (true) {
    u16 displacement = metadata_[candidate] & g_vt_displacement_mask;
    if (displacement > displacement_to_empty) {
      return candidate;
    }
    candidate = (home_bucket + vt_quadratic(displacement)) & buckets_mask_;
  }
}

SetVItr SetV::insert_raw(void* key, bool unique, bool replace) {
  u64    hash        = vtable_->hash(key);
  u16    hashfrag    = vt_hashfrag(hash);
  size_t home_bucket = hash & buckets_mask_;

  if (!(metadata_[home_bucket] & g_vt_in_home_bucket_mask)) {
    if ((key_count_ + 1 > bucket_count() * g_vt_max_load) ||
        (metadata_[home_bucket] != g_vt_empty && !evict(home_bucket))) {
      return end();
    }

    vtable_->move_key(get_bucket(home_bucket), key);
    metadata_[home_bucket] = hashfrag | g_vt_in_home_bucket_mask | g_vt_displacement_mask;
    ++key_count_;
    SetVItr itr;
    itr.data_         = get_bucket(home_bucket);
    itr.metadatum_    = metadata_ + home_bucket;
    itr.metadata_end_ = metadata_ + buckets_mask_ + 1;
    itr.home_bucket_  = home_bucket;
    itr.vtable_       = vtable_;
    return itr;
  }

  if (!unique) {
    size_t bucket = home_bucket;
    while (true) {
      if ((metadata_[bucket] & g_vt_hash_frag_mask) == hashfrag &&
          vtable_->equals_key(get_bucket(bucket), key)) {
        if (replace) {
          vtable_->destroy_key(get_bucket(bucket));
          vtable_->move_key(get_bucket(bucket), key);
        }
        SetVItr itr;
        itr.data_         = get_bucket(bucket);
        itr.metadatum_    = metadata_ + bucket;
        itr.metadata_end_ = metadata_ + buckets_mask_ + 1;
        itr.home_bucket_  = home_bucket;
        itr.vtable_       = vtable_;
        return itr;
      }
      u16 displacement = metadata_[bucket] & g_vt_displacement_mask;
      if (displacement == g_vt_displacement_mask) {
        break;
      }
      bucket = (home_bucket + vt_quadratic(displacement)) & buckets_mask_;
    }
  }

  size_t empty;
  u16    displacement;
  if ((key_count_ + 1 > bucket_count() * g_vt_max_load) ||
      !find_first_empty(home_bucket, empty, displacement)) {
    return end();
  }

  size_t prev = find_insert_location_in_chain(home_bucket, displacement);

  vtable_->move_key(get_bucket(empty), key);
  metadata_[empty] = hashfrag | (metadata_[prev] & g_vt_displacement_mask);
  metadata_[prev]  = (metadata_[prev] & ~g_vt_displacement_mask) | displacement;
  ++key_count_;
  SetVItr itr;
  itr.data_         = get_bucket(empty);
  itr.metadatum_    = metadata_ + empty;
  itr.metadata_end_ = metadata_ + buckets_mask_ + 1;
  itr.home_bucket_  = home_bucket;
  itr.vtable_       = vtable_;
  return itr;
}

bool SetV::evict(size_t bucket) {
  size_t home_bucket = vtable_->hash(get_bucket(bucket)) & buckets_mask_;
  size_t prev        = home_bucket;
  while (true) {
    size_t next = (home_bucket + vt_quadratic(metadata_[prev] & g_vt_displacement_mask)) &
                  buckets_mask_;
    if (next == bucket) {
      break;
    }
    prev = next;
  }

  metadata_[prev] = (metadata_[prev] & ~g_vt_displacement_mask) |
                    (metadata_[bucket] & g_vt_displacement_mask);

  size_t empty;
  u16    displacement;
  if (!find_first_empty(home_bucket, empty, displacement)) {
    return false;
  }

  prev = find_insert_location_in_chain(home_bucket, displacement);

  vtable_->move_key(get_bucket(empty), get_bucket(bucket));
  metadata_[empty] = (metadata_[bucket] & g_vt_hash_frag_mask) |
                     (metadata_[prev] & g_vt_displacement_mask);
  metadata_[prev] = (metadata_[prev] & ~g_vt_displacement_mask) | displacement;
  return true;
}

bool SetV::rehash(size_t bucket_count) {
  while (true) {
    SetV new_table;
    new_table.key_count_    = 0;
    new_table.buckets_mask_ = bucket_count - 1;
    new_table.buckets_      = nullptr;
    new_table.metadata_     = nullptr;
    new_table.vtable_       = vtable_;

    void* allocation = malloc(new_table.total_alloc_size());
    if (!allocation) {
      return false;
    }

    new_table.buckets_ = (u8*)allocation;
    new_table.metadata_ =
        (u16*)((unsigned char*)allocation + new_table.metadata_offset());
    memset(new_table.metadata_, 0x00, (bucket_count + 4) * sizeof(u16));
    new_table.metadata_[bucket_count] = 0x01;

    for (size_t bucket = 0; bucket < this->bucket_count(); ++bucket) {
      if (metadata_[bucket] != g_vt_empty) {
        SetVItr itr = new_table.insert_raw(get_bucket(bucket), true, false);
        if (itr.is_end()) {
          break;
        }
      }
    }

    if (new_table.key_count_ < key_count_) {
      new_table.destroy();
      // free(new_table.buckets);
      bucket_count *= 2;
      continue;
    }

    if (buckets_mask_) {
      destroy();
      // free(buckets);
    }

    *this = new_table;
    return true;
  }
}

bool SetV::reserve(size_t size) {
  size_t a_bucket_count = vt_min_bucket_count_for_size(size);
  if (a_bucket_count <= bucket_count()) {
    return true;
  }
  return rehash(a_bucket_count);
}

bool SetV::shrink() {
  size_t a_bucket_count = vt_min_bucket_count_for_size(key_count_);
  if (a_bucket_count == bucket_count()) {
    return true;
  }
  if (a_bucket_count == 0) {
    destroy();
    return true;
  }
  return rehash(a_bucket_count);
}

#pragma clang diagnostic pop
