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
  Result parse_telegram(char *buffer, size_t buffer_size) {
    reset_state_(buffer, buffer_size);
    if (reinterpret_cast<uintptr_t>(buffer) % 2 != 0) {
      return Result(Status::BUFFER_NOT_ALIGNED, nullptr, 0);
    }
    read_header_();
    if (status_ != Status::OK) {
      return Result(status_, nullptr, 0);
    }
    uint8_t *num_objects = write_<uint8_t>(0);
    if ((write_pos_ - buffer) % 2 != 0) {
      // Add padding to align header to 2 bytes
      write_('\0');
    }
    while (status_ == Status::OK) {
      next_char_();
      if (eod_) {
        break;
      } else if (std::isspace(ch_) != 0) {
        continue;
      } else if (ch_ == '!') {
        // CRC-16 checksum marker
        uint16_t crc = read_crc_();
        if (crc != crc_calculator_.crc()) {
          status_ = Status::CRC_CHECK_FAILED;
        }
      } else if (std::isdigit(ch_) != 0) {
        // OBIS code
        if (*num_objects == MAX_NUM_OBJECTS) {
          status_ = Status::TOO_MANY_OBJECTS;
        } else {
          read_object_();
          ++(*num_objects);
        }
      } else {
        status_ = Status::PARSING_FAILED;
      }
    }
    return Result(status_, buffer, write_pos_ - buffer);
  }

 protected:
  const char &next_char_(bool update_crc = true) {
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

  template<typename T> T *write_(const T &val) {
    if ((read_pos_ - write_pos_) < static_cast<ptrdiff_t>(sizeof(T))) {
      status_ = Status::WRITE_OVERFLOW;
      return nullptr;
    }
    T *item_pos = reinterpret_cast<T *>(write_pos_);
    *item_pos = val;
    write_pos_ += sizeof(T);
    return item_pos;
  }

  void reset_state_(char *buffer, size_t buffer_size) {
    read_pos_ = write_pos_ = buffer;
    buffer_end_ = &buffer[buffer_size];
    status_ = Status::OK;
    crc_calculator_.reset();
    eod_ = buffer_size == 0;
  }

  void read_header_() {
    if (next_char_() != '/') {
      status_ = Status::START_NOT_FOUND;
      return;
    }
    const char *const start = write_pos_;
    do {
      if (next_char_() == eod_) {
        status_ = Status::PARSING_FAILED;
      } else if (ch_ == '\r') {
        if (next_char_() != '\n') {
          status_ = Status::PARSING_FAILED;
        }
        write_('\0');
        if (write_pos_ - start > MAX_HEADER_SIZE) {
          status_ = Status::HEADER_TOO_LONG;
        }
        break;
      } else {
        write_(ch_);
      }
    } while (status_ == Status::OK);
  }

  ObisCode read_obis_code_() {
    size_t part = 0;
    uint8_t cur = 0;
    ObisCode obis_code{0, 0, 0, 0, 0};
    while (status_ == Status::OK) {
      if (std::isdigit(ch_) != 0) {
        const uint8_t val = ch_ - '0';
        if (cur > 25 || (cur == 25 && val > 5)) {
          status_ = Status::INVALID_OBIS_CODE;
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
          status_ = Status::INVALID_OBIS_CODE;
        }
        break;
      }
      next_char_();
    }
    return obis_code;
  }

  void read_object_() {
    auto obis_code = read_obis_code_();
    if (status_ != Status::OK) {
      return;
    }
    auto *header = write_<Header>(Header{obis_code, 0, 0});
    while (status_ == Status::OK) {
      if (eod_) {
        status_ = Status::PARSING_FAILED;
      } else if (ch_ == '(') {
        ++(header->num_values);
        while (next_char_() != ')' && !eod_) {
          write_(ch_);
        }
        write_('\0');
      } else if (ch_ == '\r') {
        ptrdiff_t object_size = write_pos_ - reinterpret_cast<char *>(header);
        if (next_char_() != '\n') {
          status_ = Status::PARSING_FAILED;
        } else if (object_size % 2 != 0) {
          write_('\0');
          ++object_size;
        }
        if (object_size > MAX_OBJECT_SIZE) {
          status_ = Status::OBJECT_TOO_LONG;
        } else {
          header->object_size = static_cast<uint16_t>(object_size);
        }
        return;
      }
      next_char_();
    };
  }

  uint16_t read_crc_() {
    uint16_t checksum = 0;
    for (uint32_t i = 0; i < 4; ++i) {
      next_char_(false);
      uint16_t value;
      if ('0' <= ch_ && ch_ <= '9') {
        value = ch_ - '0';
      } else if ('A' <= ch_ && ch_ <= 'F') {
        value = ch_ - 'A' + 10;
      } else if ('a' <= ch_ && ch_ <= 'f') {
        value = ch_ - 'a' + 10;
      } else {
        status_ = Status::INVALID_CRC;
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
  Status status_ = Status::OK;
};

using Parser = BaseParser<Crc16Calculator>;

}  // namespace efs
}  // namespace esphome
