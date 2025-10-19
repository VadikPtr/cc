#pragma once
#include "cc/arr-view.hpp"
#include "cc/common.hpp"
#include "cc/error.hpp"
#include "cc/fs.hpp"
#include "cc/str.hpp"

class BStreamWriter {
  File* file_;

 public:
  BStreamWriter(File& file) : file_(&file) {}
  BStreamWriter(const BStreamWriter&)            = default;
  BStreamWriter& operator=(const BStreamWriter&) = default;

  bool try_write(const void* from, size_t size) {
    return file_->try_write_bytes(ArrView<u8>((u8*)from, size));
  }

  void write(const void* from, size_t size) {
    if (!try_write(from, size)) {
      throw Err("Write fail");
    }
  }

  template <typename T>
  void write(const T& value) {
    write(&value, sizeof(T));
  }

  template <typename T>
  void write_arr(ArrView<T> arr) {
    write(arr.data(), arr.byte_size());
  }

  void write(const size_t& value) {
    u64 conv = (u64)value;
    write<u64>(conv);
  }
  void write(StrView str) {
    write(str.size());
    write_arr(to_bytes(str));
  }
};

class BStreamReader {
  ArrView<u8> data_;

 public:
  BStreamReader(ArrView<u8> data) : data_(data) {}
  BStreamReader(const BStreamReader&)            = default;
  BStreamReader& operator=(const BStreamReader&) = default;

  bool try_read(void* to, size_t size) {
    if (size == 0) {
      return true;
    }
    if (data_.size() < size) {
      return false;
    }
    memcpy(to, data_.data(), size);
    data_ = data_.sub(size);
    return true;
  }

  void read(void* to, size_t size) {
    if (!try_read(to, size)) {
      throw Err("No enough data in bstream");
    }
  }

  template <typename T>
  T read() {
    T value;
    read(&value, sizeof(T));
    return value;
  }

  template <typename T>
  void read_arr(ArrView<T> arr) {
    read(arr.data(), arr.byte_size());
  }

  u8     read_u8() { return read<u8>(); }
  u16    read_u16() { return read<u16>(); }
  u32    read_u32() { return read<u32>(); }
  u64    read_u64() { return read<u64>(); }
  s8     read_s8() { return read<s8>(); }
  s16    read_s16() { return read<s16>(); }
  s32    read_s32() { return read<s32>(); }
  s64    read_s64() { return read<s64>(); }
  size_t read_size() { return (size_t)read<u64>(); }
  Str    read_str() {
    Str res(read_size());
    read_arr(to_bytes(res));
    return res;
  }
};
