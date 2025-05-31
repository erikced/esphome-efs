#pragma once
#include <iterator>
#include <string_view>

#include "header.h"
#include "obis_code.h"
#include "object.h"

namespace esphome {
namespace efs {

class ObjectIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using value_type = const Object;
  using difference_type = std::ptrdiff_t;
  using pointer = const Object *;
  using reference = const Object &;

  ObjectIterator()
      : buffer_(nullptr), buffer_end_(nullptr), num_objects_(0), current_{{0, 0, 0, 0, 0}, 0, std::string_view()} {};

  ObjectIterator(const char *buffer, size_t buffer_size)
      : buffer_(buffer),
        buffer_end_(buffer + buffer_size),
        num_objects_(0),
        current_{{0, 0, 0, 0, 0}, 0, std::string_view()} {
    if (buffer_ == nullptr || buffer_size == 0) {
      buffer_ = nullptr;
      return;
    }

    // Handle identification string
    const auto identification_size = strnlen(buffer_, buffer_size) + 1;
    current_ = Object{ObisCode(0, 0, 0, 0, 0), 1, std::string_view(buffer_, identification_size)};
    buffer_ += identification_size;

    // Get number of objects
    num_objects_ = *reinterpret_cast<const uint8_t *>(buffer_);
    ++buffer_;

    // Align to 2-byte boundary
    if (reinterpret_cast<uintptr_t>(buffer_) % 2 != 0) {
      ++buffer_;
    }
  }

  reference operator*() const { return current_; }
  pointer operator->() const { return &current_; }

  ObjectIterator &operator++() {
    if (buffer_ == nullptr || num_objects_ == 0 || buffer_end_ - buffer_ <= static_cast<ptrdiff_t>(HEADER_SIZE)) {
      buffer_ = nullptr;
      return *this;
    }

    const Header *header = reinterpret_cast<const Header *>(buffer_);
    if (header->object_size < HEADER_SIZE || buffer_end_ - buffer_ < header->object_size) {
      buffer_ = nullptr;
      return *this;
    }

    current_ = Object{header->obis_code, header->num_values,
                      std::string_view(&buffer_[HEADER_SIZE], header->object_size - HEADER_SIZE)};
    buffer_ += header->object_size;
    --num_objects_;

    return *this;
  }

  bool operator==(const ObjectIterator &other) const { return buffer_ == other.buffer_; }

  bool operator!=(const ObjectIterator &other) const { return !(*this == other); }

 private:
  const char *buffer_;
  const char *buffer_end_;
  uint8_t num_objects_;
  Object current_;
};

}  // namespace efs
}  // namespace esphome
