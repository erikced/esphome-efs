#pragma once
#include <cstdint>
#include <cstddef>

#include "crc16.h"
#include "obis_code.h"

namespace esphome {
namespace efs {
class Parser {
 public:
  enum Status {
    Ok,
    StartNotFound,
    WriteOverflow,
    InvalidObisCode,
    ParsingFailed,
    ObjectTooLong,
    InvalidCrc,
    CrcCheckFailed,
  };
  Parser(){};
  Status parse_telegram(char *buffer, size_t buffer_size);

 protected:
  const char &next_char(bool update_crc = true) {
    if (read_pos != buffer_end && ++read_pos != buffer_end) {
      ch = *read_pos;
      if (update_crc) {
        crc_calculator.update(ch);
      }
    } else {
      ch = '\0';
    }
    return ch;
  };

  template<typename T> T *write(const T &val) {
    if ((read_pos - write_pos) < static_cast<ptrdiff_t>(sizeof(T))) {
      status = Status::WriteOverflow;
      return nullptr;
    }
    T *item_pos = reinterpret_cast<T *>(write_pos);
    *item_pos = val;
    write_pos += sizeof(T);
    return item_pos;
  }
  ObisCode read_obis_code();
  uint16_t read_crc();
  void update_crc(const char &ch);
  void reset_state(char *buffer, size_t buffer_size);
  bool is_whitespace() const;
  bool next_object();
  void read_object();

  Crc16Calculator crc_calculator;

 private:
  uint16_t crc = 0;
  const char *read_pos;
  char *write_pos;
  char ch;
  char *buffer = nullptr;
  const char *buffer_end = nullptr;
  Status status = Status::Ok;
};
}  // namespace efs
}  // namespace esphome
