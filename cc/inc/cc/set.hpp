#pragma once
#include "cc/common.hpp"
#include "cc/hash.hpp"

// verstable adaptation for c++.
// see LICENSE.verstable for more information.

namespace details {
  class SetV;

  struct SetVTable {
    size_t bucket_size;  // sizeof real Bucket struct = sizeof Key
    u64 (*hash)(void* key);
    bool (*equals_key)(void* a, void* b);
    void (*move_key)(void* dst, void* src);
    void (*copy_key)(void* dst, void* src);
    // void (*move_bucket)(void* dst, void* src);
    void (*destroy_key)(void* key);
  };

  class SetVItr {
   protected:
    void*            data_         = nullptr;
    u16*             metadatum_    = nullptr;
    u16*             metadata_end_ = nullptr;
    size_t           home_bucket_  = 0;
    const SetVTable* vtable_       = nullptr;

    friend SetV;

   public:
    bool     is_end() const { return metadatum_ == metadata_end_; }
    SetVItr& operator++();

   private:
    void fast_forward();
  };

  class SetV {
   protected:
    size_t           key_count_    = 0;
    size_t           buckets_mask_ = 0;
    void*            buckets_      = nullptr;
    u16*             metadata_     = nullptr;
    const SetVTable* vtable_       = nullptr;

   public:
    bool init(const SetV& other);
    void init(SetV&& other);
    void init(const SetVTable* a_vtable);
    void clear();
    void destroy();

    SetVItr        insert(void* key);
    SetVItr        get(void* key);
    bool           erase(void* key);
    SetVItr        begin() const;
    static SetVItr end() { return {}; }
    size_t         size() const;
    bool           reserve(size_t size);
    bool           shrink();

   private:
    size_t  total_alloc_size() const;
    size_t  bucket_count() const;
    size_t  metadata_offset() const;
    u8*     get_bucket(size_t idx) const;
    bool    find_first_empty(size_t home_bucket, /*out*/ size_t& empty,
                             /*out*/ u16& displacement) const;
    size_t  find_insert_location_in_chain(size_t home_bucket,
                                          u16    displacement_to_empty) const;
    bool    erase_itr_raw(SetVItr itr);
    SetVItr insert_raw(void* key, bool unique, bool replace);
    bool    evict(size_t bucket);
    bool    rehash(size_t bucket_count);
  };
}  // namespace details


template <typename TKey>
class Set final : details::SetV {
 public:
  using Key = TKey;

  class Iterator final : details::SetVItr {
   public:
    Iterator() = default;
    explicit Iterator(const SetVItr& base) noexcept : SetVItr(base) {}

    const TKey& key() const {
      assert(!is_end());
      return *static_cast<const Key*>(data_);
    }

    operator bool() const { return !is_end(); }
    const TKey& operator*() const { return key(); }

    bool operator==(const Iterator& o) const {
      return (is_end() && o.is_end()) || data_ == o.data_;
    }
    bool operator!=(const Iterator& o) const { return !(*this == o); }

    using SetVItr::operator++;
  };

  Set() {
    static details::SetVTable vtable{
        .bucket_size = sizeof(TKey),
        .hash        = v_hash,
        .equals_key  = v_equals_key,
        .move_key    = v_move_key,
        .copy_key    = v_copy_key,
        .destroy_key = v_destroy_key,
    };
    init(&vtable);
  }

  ~Set() { destroy(); }

  Set(const Set& other) { init(other); }

  Set& operator=(const Set& other) {
    if (this != &other) {
      destroy();
      init(other);
    }
    return *this;
  }

  Set(Set&& other) noexcept : SetV() { init(static_cast<SetV&&>(other)); }

  Set& operator=(Set&& other) noexcept {
    if (this != &other) {
      init(static_cast<SetV&&>(other));
    }
    return *this;
  }

  Iterator insert(TKey&& key) { return Iterator(SetV::insert(&key)); }
  Iterator find(const TKey& key) { return Iterator(SetV::get(&const_cast<TKey&>(key))); }
  bool     erase(const TKey& key) { return SetV::erase(&const_cast<TKey&>(key)); }
  Iterator begin() const { return Iterator(SetV::begin()); }
  static constexpr Iterator end() { return Iterator(); }

  using SetV::clear;
  using SetV::reserve;
  using SetV::shrink;
  using SetV::size;

 private:
  static u64  v_hash(void* key) { return cc::hash<TKey>(*static_cast<TKey*>(key)); }
  static bool v_equals_key(void* a, void* b) {
    return cc::equals<TKey>(*static_cast<TKey*>(a), *static_cast<TKey*>(b));
  }
  static void v_move_key(void* dst, void* src) {
    new (dst) TKey(move(*static_cast<TKey*>(src)));
  }
  static void v_copy_key(void* dst, void* src) {
    new (dst) TKey(*static_cast<const TKey*>(src));
  }
  static void v_destroy_key(void* dst) { static_cast<TKey*>(dst)->~TKey(); }
};
