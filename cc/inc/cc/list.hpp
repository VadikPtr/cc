#pragma once
#include "cc/common.hpp"

// double linked list
template <typename T>
class List {
 public:
  struct Node {
    T     value;
    Node* next = nullptr;
    Node* prev = nullptr;

    Node(T value) : value(move(value)) {}
  };

  class Iterator {
    Node* current_ = nullptr;
    Node* next_    = nullptr;

   public:
    Iterator(Node* current = nullptr)
        : current_(current), next_(current ? current->next : nullptr) {}

    T*    operator->() const { return &current_->value; }
    T&    operator*() const { return current_->value; }
    Node* node() const { return current_; }

    Iterator& operator++() {
      assert(current_ != nullptr);
      current_ = next_;
      if (current_) {
        next_ = current_->next;
      } else {
        next_ = nullptr;
      }
      return *this;
    }

    bool operator==(const Iterator& o) const { return o.current_ == current_; }
  };

  class ConstIterator {
    const Node* current_ = nullptr;

   public:
    ConstIterator(Node* current = nullptr) : current_(current) {}

    const T* operator->() const { return &current_->value; }
    const T& operator*() const { return current_->value; }

    ConstIterator& operator++() {
      assert(current_ != nullptr);
      current_ = current_->next;
      return *this;
    }

    bool operator==(const ConstIterator& o) const { return o.current_ == current_; }
  };

 private:
  Node*  front_ = nullptr;
  Node*  back_  = nullptr;
  size_t size_  = 0;

 public:
  List() = default;

  List(const List& other) { copy(other, *this); }

  List& operator=(const List& other) {
    if (this != &other) {
      copy(other, *this);
    }
    return *this;
  }

  List(List&& other) {
    swap(front_, other.front_);
    swap(back_, other.back_);
    swap(size_, other.size_);
  }

  List& operator=(List&& other) {
    if (this != &other) {
      swap(front_, other.front_);
      swap(back_, other.back_);
      swap(size_, other.size_);
    }
    return *this;
  }

  ~List() { clear(); }

  bool          empty() const { return size_ == 0; }
  size_t        size() const { return size_; }
  Iterator      begin() { return Iterator{front_}; }
  Iterator      end() { return Iterator{}; }
  ConstIterator begin() const { return {front_}; }
  ConstIterator end() const { return {}; }

  void clear() {
    while (front_) {
      auto* next = front_->next;
      delete front_;
      front_ = next;
    }
    front_ = nullptr;
    back_  = nullptr;
    size_  = 0;
  }

  T remove(const Iterator& it) {
    auto* node = it.node();
    if (node->prev) {
      node->prev->next = node->next;
    } else {
      front_ = node->next;  // it is front
    }
    if (node->next) {
      node->next->prev = node->prev;
    } else {
      back_ = node->prev;  // it is back
    }
    T value(move(node->value));
    delete node;
    return value;
  }

  T& push_front(T value) {
    auto* new_front = new Node(move(value));
    new_front->next = front_;
    front_          = new_front;
    if (!back_) {  // list empty
      back_ = front_;
    } else {
      front_->next->prev = front_;
    }
    ++size_;
    return new_front->value;
  }

  T& push_back(T value) {
    auto* new_back = new Node(move(value));
    new_back->prev = back_;
    back_          = new_back;
    if (!front_) {  // list empty
      front_ = back_;
    } else {
      back_->prev->next = back_;
    }
    ++size_;
    return new_back->value;
  }

  T pop_front() {
    assert(front_);
    auto* node = front_;
    front_     = front_->next;
    if (!front_) {  // list empty
      back_ = nullptr;
    } else {
      front_->prev = nullptr;
    }
    --size_;
    T value(move(node->value));
    delete node;
    return value;
  }

  T pop_back() {
    assert(back_);
    auto* node = back_;
    back_      = back_->prev;
    if (!back_) {  // list empty
      front_ = nullptr;
    } else {
      back_->next = nullptr;
    }
    --size_;
    T value(move(node->value));
    delete node;
    return value;
  }

 private:
  static void copy(const List& from, List& to) {
    assert(to.front_ == nullptr);
    for (const auto& v : from) {
      to.push_back(v);
    }
  }
};
