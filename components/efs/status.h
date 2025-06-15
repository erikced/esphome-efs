#pragma once

namespace esphome {
namespace efs {
enum class Status {
  OK,
  BUFFER_NOT_ALIGNED,
  START_NOT_FOUND,
  WRITE_OVERFLOW,
  INVALID_OBIS_CODE,
  PARSING_FAILED,
  HEADER_TOO_LONG,
  OBJECT_TOO_LONG,
  TOO_MANY_OBJECTS,
  INVALID_CRC,
  CRC_CHECK_FAILED,
};
}  // namespace efs
}  // namespace esphome
