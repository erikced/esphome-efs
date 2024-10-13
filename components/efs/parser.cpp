#include "parser.h"

#include "header.h"
#include "obis_code.h"

#include <array>
#include <limits>
#include <ranges>
#include <string.h>

namespace esphome {
namespace efs {

Parser::Status Parser::parse_telegram(char *buffer, size_t buffer_size) {
  reset_state(buffer, buffer_size);
  if (ch != '/') {
    return Status::StartNotFound;
  }
  next_char();
  while (ch != '\r' && ch != '\n' && ch != '\0') {
    write(ch);
    next_char();
  }
  write('\0');
  while (next_char() != '\n') {
  };
  uint8_t *num_objects = write<uint8_t>(0);
  while (status == Status::Ok && next_object()) {
    if (ch == '!') {
      write('\0');
      uint16_t crc = read_crc();
      if (crc != crc_calculator.crc) {
        status = Status::CrcCheckFailed;
      }
      break;
    } else {
      read_object();
      ++(*num_objects);
    }
  }
  return status;
}

void Parser::reset_state(char *buffer, size_t buffer_size) {
  this->buffer = buffer;
  this->buffer_end = &buffer[buffer_size];
  this->read_pos = this->buffer;
  this->write_pos = this->buffer;
  this->ch = read_pos != buffer_end ? *read_pos : '\0';
  this->status = Status::Ok;
  crc_calculator.reset();
  crc_calculator.update(ch);
}

ObisCode Parser::read_obis_code() {
  size_t part = 0;
  ObisCode obis_code{0, 0, 0, 0, 0, 0};
  while (status == Status::Ok) {
    if (ch >= '0' && ch <= '9') {
      const uint8_t val = ch - '0';
      if (obis_code.value[part] > 25 || (obis_code.value[part] == 25 && val > 5)) {
        status = Status::InvalidObisCode;
        break;
      }
      obis_code.value[part] = obis_code.value[part] * 10 + val;
    } else if ((ch == '-' || ch == ':' || ch == '.' || ch == '*') && part <= 4) {
      ++part;
    } else {
      // The final part of the obis code is usually omitted
      if (part == 4) {
        obis_code.value[5] = 255u;
      } else {
        status = Status::InvalidObisCode;
      }
      break;
    }
    next_char();
  }
  return obis_code;
}

void Parser::read_object() {
  auto obis_code = read_obis_code();
  if (status != Status::Ok) {
    return;
  }
  auto *header = write<Header>(Header{obis_code, 0, 0});
  while (status == Status::Ok) {
    if (ch == '\0') {
      status = Status::ParsingFailed;
    } else if (ch == '(') {
      ++(header->num_values);
      while (next_char() != ')' && ch != '\0') {
        write(ch);
      }
      write('\0');
    } else if (ch == '\r' || ch == '\n') {
      if (header->num_values == 0) {
        write('\0');
      }
      uint16_t offset = write_pos - reinterpret_cast<char *>(header);
      if (offset > std::numeric_limits<uint8_t>::max()) {
        status = Status::ObjectTooLong;
      } else {
        header->object_size = static_cast<uint8_t>(offset);
      }
      return;
    }
    next_char();
  };
}

uint16_t Parser::read_crc() {
  uint16_t checksum = 0;
  for (uint32_t i = 0; i < 4; ++i) {
    next_char(false);
    uint16_t value;
    if ('0' <= ch && ch <= '9') {
      value = ch - '0';
    } else if ('A' <= ch && ch <= 'F') {
      value = ch - 'A' + 10;
    } else if ('a' <= ch && ch <= 'f') {
      value = ch - 'a' + 10;
    } else {
      status = Status::InvalidCrc;
      return 0;
    }
    checksum = (checksum << 4) + value;
  }
  return checksum;
}

bool Parser::is_whitespace() const { return ch == '\r' || ch == '\n' || ch == ' '; }

bool Parser::next_object() {
  bool whitespace_found = is_whitespace();
  while (true) {
    next_char();
    if (ch == '\0') {
      return false;
    } else if (whitespace_found && !is_whitespace()) {
      return true;
    } else if (!whitespace_found && is_whitespace()) {
      whitespace_found = true;
    }
  }
}

}  // namespace efs
}  // namespace esphome
