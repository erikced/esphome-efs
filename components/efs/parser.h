#pragma once
#include <cstdint>
#include <cstddef>

#include "crc16.h"
#include "header.h"
#include "obis_code.h"

namespace esphome {
namespace efs {
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

const uint32_t MAX_OBJECT_SIZE = 8192;

template<typename CrcCalculator> class BaseParser {
 public:
  Status parse_telegram(char *buffer, size_t buffer_size) {
    reset_state(buffer, buffer_size);
    if (next_char() != '/') {
      return Status::StartNotFound;
    }
    next_char();
    while (ch_ != '\r' && ch_ != '\n' && ch_ != '\0') {
      write(ch_);
      next_char();
    }
    write('\0');
    while (ch_ != '\n' && ch_ != '\0') {
      next_char();
    }
    uint8_t *num_objects = write<uint8_t>(0);
    while (status_ == Status::Ok && next_object()) {
      if (ch_ == '!') {
        write('\0');
        uint16_t crc = read_crc();
        if (crc != crc_calculator_.crc()) {
          status_ = Status::CrcCheckFailed;
        }
        break;
      } else {
        read_object();
        ++(*num_objects);
      }
    }
    return status_;
  }

 protected:
  const char &next_char(bool update_crc = true) {
    eod_ |= read_pos_ == buffer_end_;
    if (!eod_) {
      ch_ = *read_pos_++;
      if (update_crc) {
        crc_calculator_.update(ch_);
      }
      eod_ = ch_ == '\0';
    } else {
      ch_ = '\0';
    }
    return ch_;
  }

  template<typename T> T *write(const T &val) {
    if ((read_pos_ - write_pos_) < static_cast<ptrdiff_t>(sizeof(T))) {
      status_ = Status::WriteOverflow;
      return nullptr;
    }
    T *item_pos = reinterpret_cast<T *>(write_pos_);
    *item_pos = val;
    write_pos_ += sizeof(T);
    return item_pos;
  }

  void reset_state(char *buffer, size_t buffer_size) {
    read_pos_ = write_pos_ = buffer;
    buffer_end_ = &buffer[buffer_size];
    status_ = Status::Ok;
    crc_calculator_.reset();
    eod_ = buffer_size == 0;
  }
  }

  ObisCode read_obis_code() {
    size_t part = 0;
    uint8_t cur = 0;
    ObisCode obis_code{0, 0, 0, 0, 0};
    while (status_ == Status::Ok) {
      if (ch_ >= '0' && ch_ <= '9') {
        const uint8_t val = ch_ - '0';
        if (cur > 25 || (cur == 25 && val > 5)) {
          status_ = Status::InvalidObisCode;
          break;
        }
        cur = cur * 10 + val;
      } else if ((ch_ == '-' || ch_ == ':' || ch_ == '.' || ch_ == '*') && part <= 4) {
        obis_code[part] = cur;
        cur = 0;
        ++part;
      } else {
        // The final part of the obis code is usually omitted
        if (part == 4) {
            obis_code[4] = cur;
        } else if (part != 5 || (part == 5 && cur != 255)) {
          // Reading 6-part obis codes is supported but the 6th part must be 255
          // since only 5 parts are stored.
          status_ = Status::InvalidObisCode;
        }
        break;
      }
      next_char();
    }
    return obis_code;
  }

  void read_object() {
    auto obis_code = read_obis_code();
    if (status_ != Status::Ok) {
      return;
    }
    auto *header = write<Header>(Header{obis_code, 0, 0});
    while (status_ == Status::Ok) {
      if (eod_) {
        status_ = Status::ParsingFailed;
      } else if (ch_ == '(') {
        ++(header->num_values);
        while (next_char() != ')' && !eod_) {
          write(ch_);
        }
        write('\0');
      } else if (ch_ == '\r' || ch_ == '\n') {
        uint32_t offset = write_pos_ - reinterpret_cast<const char *>(header);
        if (offset > MAX_OBJECT_SIZE) {
          status_ = Status::ObjectTooLong;
        } else {
          header->object_size = static_cast<uint16_t>(offset);
        }
        return;
      }
      next_char();
    };
  }

  uint16_t read_crc() {
    uint16_t checksum = 0;
    for (uint32_t i = 0; i < 4; ++i) {
      next_char(false);
      uint16_t value;
      if ('0' <= ch_ && ch_ <= '9') {
        value = ch_ - '0';
      } else if ('A' <= ch_ && ch_ <= 'F') {
        value = ch_ - 'A' + 10;
      } else if ('a' <= ch_ && ch_ <= 'f') {
        value = ch_ - 'a' + 10;
      } else {
        status_ = Status::InvalidCrc;
        return 0;
      }
      checksum = (checksum << 4) + value;
    }
    return checksum;
  }

  bool is_whitespace() const { return ch_ == '\r' || ch_ == '\n' || ch_ == ' '; }

  bool next_object() {
    bool whitespace_found = is_whitespace();
    while (true) {
      next_char();
      if (eod_) {
        return false;
      } else if (whitespace_found && !is_whitespace()) {
        return true;
      } else if (!whitespace_found && is_whitespace()) {
        whitespace_found = true;
      }
    }
  }

  CrcCalculator crc_calculator_{};

 private:
  const char *read_pos_ = nullptr;
  char *write_pos_ = nullptr;
  char ch_ = '\0';
  bool eod_ = false;
  const char *buffer_end_ = nullptr;
  Status status_ = Status::Ok;
};

using Parser = BaseParser<Crc16Calculator>;
}  // namespace efs
}  // namespace esphome
