#pragma once
#include "cc/common.hpp"
#include "cc/arr.hpp"

template <typename TKey, typename TValue>
class SDict {
  Arr<TKey>   keys_;
  Arr<TValue> values_;
  size_t      size_ = 0;

 public:
  using Key   = TKey;
  using Value = TValue;

  void reserve(size_t size) {
    keys_.resize(size);
    values_.resize(size);
  }

  // expects key to be unique, does no checks for it
  void insert(TKey key, TValue value) {
    assert(size_ < keys_.size());
    keys_[size_]   = move(key);
    values_[size_] = move(value);
    size_++;
  }

  size_t size() const { return size_; }

  void sort() {
    if (size_ <= 1) {
      return;
    }
    for (size_t i = 0; i < size_; ++i) {
      bool swapped = false;
      for (size_t j = 0; j < size_ - i - 1; ++j) {
        if (cc::is_less<TKey>(keys_[j + 1], keys_[j])) {
          swapped = true;
          swap(keys_[j], keys_[j + 1]);
          swap(values_[j], values_[j + 1]);
        }
      }
      if (!swapped) {  // array is sorted
        return;
      }
    }
  }

  TValue& operator[](const TKey& key) {
    TValue* value = find(key);
    assert(value);
    return *value;
  }

  // returns null when not found
  // TODO: fix s32 indexing
  TValue* find(const TKey& key) {
    if (size_ == 0) {
      return nullptr;
    }
    if (size_ == 1) {
      return keys_[0] == key ? &values_[0] : nullptr;
    }
    s32 low  = 0;
    s32 high = s32(size_) - 1;
    while (low <= high) {
      s32 mid = low + (high - low) / 2;
      if (keys_[size_t(mid)] == key) {
        return &values_[size_t(mid)];
      }
      if (keys_[size_t(mid)] < key) {
        low = mid + 1;
      } else {
        high = mid - 1;
      }
    }
    return nullptr;
  }

  template <typename T>
  TValue* find_non_sorted(const T& key) {
    for (size_t i = 0; i < size_; i++) {
      if (keys_[i] == key) {
        return &values_[i];
      }
    }
    return nullptr;
  }

  template <typename T>
  const TValue* find_non_sorted(const T& key) const {
    for (size_t i = 0; i < size_; i++) {
      if (keys_[i] == key) {
        return &values_[i];
      }
    }
    return nullptr;
  }

  struct IterView {
    const TKey& key;
    TValue&     value;
  };

  class Iter {
    SDict* dict_;
    size_t index_;

   public:
    Iter(SDict* dict, size_t index) : dict_(dict), index_(index) {}

    bool operator==(const Iter& other) const {
      return dict_ == other.dict_ && index_ == other.index_;
    }
    IterView operator*() {
      return IterView(dict_->keys_[index_], dict_->values_[index_]);
    }
    Iter& operator++() {
      if (++index_ == dict_->size_) {
        index_ = 0;
        dict_  = nullptr;
      }
      return *this;
    }
  };

  Iter        begin() { return size_ > 0 ? Iter(this, 0) : end(); }
  static Iter end() { return Iter(nullptr, 0); }
};
