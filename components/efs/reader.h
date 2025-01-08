#pragma once
#include <cstddef>
#include <cstdint>
#include <string.h>

#include "header.h"

namespace esphome {
namespace efs {
class Reader {
 public:
  template<typename TFunc> void read_parsed_data(const char *buffer, size_t buffer_size, const TFunc &callback) {
    if (buffer == nullptr) {
      return;
    }
    callback(ObisCode(0, 0, 0, 0, 0), 1, buffer);
    const char *buffer_end = buffer + buffer_size;
    buffer += strnlen(buffer, buffer_size) + 1;
    uint8_t num_objects = *reinterpret_cast<const uint8_t *>(buffer);
    ++buffer;
    while (buffer_end - buffer > static_cast<ptrdiff_t>(HEADER_SIZE) && num_objects-- > 0) {
      const Header *header = reinterpret_cast<const Header *>(buffer);
      if (header->object_size < HEADER_SIZE + 1 || buffer_end - buffer < header->object_size) {
        return;
      }
      callback(header->obis_code, header->num_values, &buffer[HEADER_SIZE]);
      buffer += header->object_size;
    }
  }
};
}  // namespace efs
}  // namespace esphome
