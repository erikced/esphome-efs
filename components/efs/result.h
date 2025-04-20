#pragma once
#include <cstddef>
#include <cstdint>

#include "object.h"
#include "object_iterator.h"
#include "status.h"

namespace esphome {
namespace efs {

class Result {
 public:
  using value_type = Object;
  using const_iterator = ObjectIterator;

  Result(Status status, const char *buffer, size_t buffer_size)
      : status(status), buffer_(buffer), buffer_size_(buffer_size) {}

  const_iterator begin() const { return const_iterator(buffer_, buffer_size_); }

  const_iterator end() const { return const_iterator(); }

  const Status status;

 private:
  const char *const buffer_;
  const size_t buffer_size_;
};

}  // namespace efs
}  // namespace esphome
