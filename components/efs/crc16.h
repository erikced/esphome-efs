#pragma once
#include <array>
#include <cstdint>

namespace esphome {
namespace efs {
namespace util {
constexpr std::array<uint16_t, 256> init_table() {
  const uint16_t polynomial = 0xA001;
  std::array<uint16_t, 256> table = {0};
  uint16_t counter = 0;
  for (auto &table_value : table) {
    uint16_t value = 0;
    uint16_t temp = counter;
    for (auto j = 0; j < 8; ++j) {
      if (((value ^ temp) & 0x0001) != 0) {
        value = (value >> 1) ^ polynomial;
      } else {
        value = value >> 1;
      }
      temp = temp >> 1;
    }
    table_value = value;
    ++counter;
  }
  return table;
}
}  // namespace util

class Crc16Calculator {
 public:
  void update(const char &ch) {
    const auto index = static_cast<uint8_t>(crc_ ^ ch);
    crc_ = (crc_ >> 8) ^ TABLE[index];
  };

  uint16_t crc() { return crc_; };
  void reset() { crc_ = 0; };

 protected:
  uint16_t crc_ = 0;
  static constexpr auto TABLE = util::init_table();
};

}  // namespace efs
}  // namespace esphome
