#pragma once
#include <cstring>
#include <iterator>
#include <tuple>

namespace esphome {
namespace efs {

class ValueIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using value_type = const std::tuple<const char *, size_t>;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  ValueIterator() : buffer_(nullptr), buffer_size_(0), remaining_values_(0), current_{nullptr, 0} {};

  ValueIterator(const char *buffer, size_t buffer_size, uint8_t num_values)
      : buffer_(buffer), buffer_size_(buffer_size), remaining_values_(num_values), current_() {
    if (buffer_ == nullptr || buffer_size == 0 || num_values == 0) {
      buffer_ = nullptr;
      return;
    }
    next_();
  }

  reference operator*() const { return current_; }
  pointer operator->() const { return &current_; }

  ValueIterator &operator++() {
    if (buffer_ == nullptr || buffer_size_ == 0 || remaining_values_ == 0) {
      buffer_ = nullptr;
      return *this;
    }

    // Skip past current string and its terminating null
    size_t value_size = std::get<1>(current_) + 1;
    if (value_size >= buffer_size_) {
      buffer_ = nullptr;
      return *this;
    }

    buffer_ += value_size;
    buffer_size_ -= value_size;

    if (--remaining_values_ > 0) {
      next_();
    } else {
      buffer_ = nullptr;
    }
    return *this;
  }

  bool operator==(const ValueIterator &other) const { return buffer_ == other.buffer_; }

  bool operator!=(const ValueIterator &other) const { return !(*this == other); }

 private:
  void next_() {
    size_t value_size = strnlen(buffer_, buffer_size_);
    if (value_size == buffer_size_) {
      buffer_ = nullptr;
      return;
    }
    current_ = {buffer_, value_size};
  }

  const char *buffer_;
  size_t buffer_size_;
  uint8_t remaining_values_;
  std::tuple<const char *, size_t> current_;
};

}  // namespace efs
}  // namespace esphome
