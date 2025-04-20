#pragma once
#include <cctype>
#include <cstddef>
#include <cstdint>

#include "crc16.h"
#include "header.h"
#include "obis_code.h"
#include "status.h"
#include "result.h"

namespace esphome {
namespace efs {
const uint32_t MAX_OBJECT_SIZE = 8192;
const uint32_t MAX_HEADER_SIZE = 256;
const uint32_t MAX_NUM_OBJECTS = 255;

template<typename CrcCalculator> class BaseParser {
 public:
    reset_state(buffer, buffer_size);
  Result parse_telegram(char *buffer, size_t buffer_size) {
    if (reinterpret_cast<uintptr_t>(buffer) % 2 != 0) {
      return Result(Status::BufferNotAligned, nullptr, 0);
    }
    read_header();
    if (status_ != Status::Ok) {
      return Result(status_, nullptr, 0);
    }
    uint8_t *num_objects = write<uint8_t>(0);
    if ((write_pos_ - buffer) % 2 != 0) {
      // Add padding to align header to 2 bytes
      write('\0');
    }
    while (status_ == Status::Ok) {
      next_char();
      if (eod_) {
        break;
      } else if (std::isspace(ch_) != 0) {
        continue;
      } else if (ch_ == '!') {
        // CRC-16 checksum marker
        uint16_t crc = read_crc();
        if (crc != crc_calculator_.crc()) {
          status_ = Status::CrcCheckFailed;
        }
      } else if (std::isdigit(ch_) != 0) {
        // OBIS code
        if (*num_objects == MAX_NUM_OBJECTS) {
          status_ = Status::TooManyObjects;
        } else {
          read_object();
          ++(*num_objects);
        }
      } else {
        status_ = Status::ParsingFailed;
      }
    }
    return Result(status_, buffer, write_pos_ - buffer);
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

  void read_header() {
    if (next_char() != '/') {
      status_ = Status::StartNotFound;
      return;
    }
    const char *const start = write_pos_;
    do {
      if (next_char() == eod_) {
        status_ = Status::ParsingFailed;
      } else if (ch_ == '\r') {
        if (next_char() != '\n') {
          status_ = Status::ParsingFailed;
        }
        write('\0');
        if (write_pos_ - start > MAX_HEADER_SIZE) {
          status_ = Status::HeaderTooLong;
        }
        break;
      } else {
        write(ch_);
      }
    } while (status_ == Status::Ok);
  }

  ObisCode read_obis_code() {
    size_t part = 0;
    uint8_t cur = 0;
    ObisCode obis_code{0, 0, 0, 0, 0};
    while (status_ == Status::Ok) {
      if (std::isdigit(ch_) != 0) {
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
      } else if (ch_ == '\r') {
        ptrdiff_t object_size = write_pos_ - reinterpret_cast<const char *>(header);
        if (next_char() != '\n') {
          status_ = Status::ParsingFailed;
        } else if (object_size % 2 != 0) {
          write('\0');
          ++object_size;
        }
        if (object_size > MAX_OBJECT_SIZE) {
          status_ = Status::ObjectTooLong;
        } else {
          header->object_size = static_cast<uint16_t>(object_size);
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
