#pragma once
#include <type_traits>
#include "cc/common.hpp"
#include "cc/hash.hpp"

// verstable adaptation for c++.
// see LICENSE.verstable for more information.

namespace details {
  class DictV;

  struct DictVTable {
    size_t bucket_size;  // sizeof real Bucket struct
    size_t key_offset;
    size_t value_offset;
    u64 (*hash)(void* key);
    bool (*equals_key)(void* a, void* b);
    void (*move_key)(void* dst, void* src);
    void (*move_value)(void* dst, void* src);
    void (*copy_key)(void* dst, void* src);
    void (*copy_value)(void* dst, void* src);
    void (*move_bucket)(void* dst, void* src);
    void (*destroy_key)(void* key);
    void (*destroy_value)(void* val);
  };

  class DictVItr {
   protected:
    void*             data_         = nullptr;
    u16*              metadatum_    = nullptr;
    u16*              metadata_end_ = nullptr;
    size_t            home_bucket_  = 0;
    const DictVTable* vtable_       = nullptr;

    friend DictV;

   public:
    bool      is_end() const { return metadatum_ == metadata_end_; }
    DictVItr& operator++();

   private:
    void fast_forward();
  };

  class DictV {
   protected:
    size_t            key_count_    = 0;
    size_t            buckets_mask_ = 0;
    void*             buckets_      = nullptr;
    u16*              metadata_     = nullptr;
    const DictVTable* vtable_       = nullptr;

   public:
    bool init(const DictV& other);
    void init(DictV&& other);
    void init(const DictVTable* a_vtable);
    void clear();
    void destroy();

    DictVItr        insert(void* key, void* value);
    DictVItr        get(void* key);
    bool            erase(void* key);
    void            erase(const DictVItr& it);
    DictVItr        begin() const;
    static DictVItr end() { return {}; }
    size_t          size() const;
    bool            reserve(size_t size);
    bool            shrink();

   private:
    size_t   total_alloc_size() const;
    size_t   bucket_count() const;
    size_t   metadata_offset() const;
    u8*      get_bucket(size_t idx) const;
    u8*      get_bucket_key(u8* b) const;
    u8*      get_bucket_value(u8* b) const;
    bool     find_first_empty(size_t home_bucket, /*out*/ size_t& empty,
                              /*out*/ u16& displacement) const;
    size_t   find_insert_location_in_chain(size_t home_bucket,
                                           u16    displacement_to_empty) const;
    bool     erase_itr_raw(DictVItr itr);
    DictVItr insert_raw(void* key, void* val, bool unique, bool replace);
    bool     evict(size_t bucket);
    bool     rehash(size_t bucket_count);
  };
}  // namespace details


template <typename TKey, typename TValue>
class Dict final : details::DictV {
 public:
  using Key   = TKey;
  using Value = TValue;

  struct Bucket {
    TKey   key;
    TValue value;
  };

  class Iterator final : details::DictVItr {
   public:
    Iterator() = default;
    explicit Iterator(const DictVItr& base) noexcept : DictVItr(base) {}

    const TKey& key() const {
      assert(!is_end());
      return static_cast<const Bucket*>(data_)->key;
    }

    TValue& value() {
      assert(!is_end());
      return static_cast<Bucket*>(data_)->value;
    }

    Bucket& operator*() const { return *static_cast<Bucket*>(data_); }
    operator bool() const { return !is_end(); }

    bool operator==(const Iterator& o) const {
      return (is_end() && o.is_end()) || data_ == o.data_;
    }
    bool operator!=(const Iterator& o) const { return !(*this == o); }

    using DictVItr::operator++;

    friend Dict;
  };

  Dict() {
    static details::DictVTable vtable{
        .bucket_size   = sizeof(Bucket),
        .key_offset    = __builtin_offsetof(Bucket, key),
        .value_offset  = __builtin_offsetof(Bucket, value),
        .hash          = v_hash,
        .equals_key    = v_equals_key,
        .move_key      = v_move_key,
        .move_value    = v_move_value,
        .copy_key      = v_copy_key,
        .copy_value    = v_copy_value,
        .move_bucket   = v_move_bucket,
        .destroy_key   = v_destroy_key,
        .destroy_value = v_destroy_value,
    };
    init(&vtable);
  }

  ~Dict() { destroy(); }

  Dict(const Dict& other) { init(other); }

  Dict& operator=(const Dict& other) {
    if (this != &other) {
      destroy();
      init(other);
    }
    return *this;
  }

  Dict(Dict&& other) noexcept : DictV() { init(static_cast<DictV&&>(other)); }

  Dict& operator=(Dict&& other) noexcept {
    if (this != &other) {
      init(static_cast<DictV&&>(other));
    }
    return *this;
  }

  Iterator insert(TKey&& key, TValue&& value) {
    return Iterator(DictV::insert(&key, &value));
  }

  Iterator find(const TKey& key) { return Iterator(DictV::get(&const_cast<TKey&>(key))); }
  bool     erase(const TKey& key) { return DictV::erase(&const_cast<TKey&>(key)); }
  void     erase(const Iterator& it) { return DictV::erase(it); }
  Iterator begin() const { return Iterator(DictV::begin()); }
  static constexpr Iterator end() { return Iterator(); }

  using DictV::clear;
  using DictV::reserve;
  using DictV::shrink;
  using DictV::size;

 private:
  static u64  v_hash(void* key) { return cc::hash<TKey>(*static_cast<TKey*>(key)); }
  static bool v_equals_key(void* a, void* b) {
    return cc::equals<TKey>(*static_cast<TKey*>(a), *static_cast<TKey*>(b));
  }
  static void v_move_key(void* dst, void* src) {
    new (dst) TKey(move(*static_cast<TKey*>(src)));
  }
  static void v_move_value(void* dst, void* src) {
    new (dst) TValue(move(*static_cast<TValue*>(src)));
  }
  static void v_copy_key(void* dst, void* src) {
    new (dst) TKey(*static_cast<const TKey*>(src));
  }
  static void v_copy_value(void* dst, void* src) {
    if constexpr (std::is_copy_constructible_v<TValue>) {
      new (dst) TValue(*static_cast<const TValue*>(src));
    } else {
      assert(false);
    }
  }
  static void v_move_bucket(void* dst, void* src) {
    new (dst) Bucket(move(*static_cast<Bucket*>(src)));
  }
  static void v_destroy_key(void* dst) { static_cast<TKey*>(dst)->~TKey(); }
  static void v_destroy_value(void* dst) { static_cast<TValue*>(dst)->~TValue(); }
};
