#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace esphome {
namespace efs {
class ObisCode {
 public:
  ObisCode(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e) : value{a, b, c, d, e} {};
  uint8_t& operator[](size_t pos) { return value[pos]; }
  const uint8_t& operator[](size_t pos) const { return value[pos]; }
  friend bool operator==(const ObisCode &lhs, const ObisCode &rhs) { return lhs.value == rhs.value; }
  friend bool operator<(const ObisCode &lhs, const ObisCode &rhs) { return lhs.value < rhs.value; }

 protected:
  std::array<uint8_t, 5> value;
};

const ObisCode ENERGY_IMPORTED(1, 0, 1, 8, 0);
const ObisCode ENERGY_IMPORTED_TARIFF1(1, 0, 1, 8, 1);
const ObisCode ENERGY_IMPORTED_TARIFF2(1, 0, 1, 8, 2);
const ObisCode ENERGY_EXPORTED(1, 0, 2, 8, 0);
const ObisCode ENERGY_EXPORTED_TARIFF1(1, 0, 2, 8, 1);
const ObisCode ENERGY_EXPORTED_TARIFF2(1, 0, 2, 8, 2);
const ObisCode REACTIVE_ENERGY_IMPORTED(1, 0, 3, 8, 0);
const ObisCode REACTIVE_ENERGY_EXPORTED(1, 0, 4, 8, 0);
const ObisCode POWER_IMPORTED(1, 0, 1, 7, 0);
const ObisCode POWER_EXPORTED(1, 0, 2, 7, 0);
const ObisCode REACTIVE_POWER_IMPORTED(1, 0, 3, 7, 0);
const ObisCode REACTIVE_POWER_EXPORTED(1, 0, 4, 7, 0);
const ObisCode POWER_IMPORTED_L1(1, 0, 21, 7, 0);
const ObisCode POWER_EXPORTED_L1(1, 0, 22, 7, 0);
const ObisCode POWER_IMPORTED_L2(1, 0, 41, 7, 0);
const ObisCode POWER_EXPORTED_L2(1, 0, 42, 7, 0);
const ObisCode POWER_IMPORTED_L3(1, 0, 61, 7, 0);
const ObisCode POWER_EXPORTED_L3(1, 0, 62, 7, 0);
const ObisCode REACTIVE_POWER_IMPORTED_L1(1, 0, 23, 7, 0);
const ObisCode REACTIVE_POWER_EXPORTED_L1(1, 0, 24, 7, 0);
const ObisCode REACTIVE_POWER_IMPORTED_L2(1, 0, 43, 7, 0);
const ObisCode REACTIVE_POWER_EXPORTED_L2(1, 0, 44, 7, 0);
const ObisCode REACTIVE_POWER_IMPORTED_L3(1, 0, 63, 7, 0);
const ObisCode REACTIVE_POWER_EXPORTED_L3(1, 0, 64, 7, 0);
const ObisCode VOLTAGE_L1(1, 0, 32, 7, 0);
const ObisCode VOLTAGE_L2(1, 0, 52, 7, 0);
const ObisCode VOLTAGE_L3(1, 0, 72, 7, 0);
const ObisCode CURRENT_L1(1, 0, 31, 7, 0);
const ObisCode CURRENT_L2(1, 0, 51, 7, 0);
const ObisCode CURRENT_L3(1, 0, 71, 7, 0);
const ObisCode CT_RATIO(1, 0, 1, 4, 2);
const ObisCode VT_RATIO(1, 0, 1, 4, 3);
}  // namespace efs
}  // namespace esphome
