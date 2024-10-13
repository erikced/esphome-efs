#pragma once
#include <cstddef>
#include <cstdint>

#include "obis_code.h"

namespace esphome {
namespace efs {
struct Header {
 public:
  ObisCode obis_code;
  uint8_t num_values;
  uint8_t object_size;  // Size of the current obj. including Header
};
const size_t HEADER_SIZE = sizeof(Header);
static_assert(HEADER_SIZE == 8, "Header size should be 8 bytes.");
}  // namespace efs
}  // namespace esphome
