#pragma once
#include <string_view>

#include "object.h"
#include "obis_code.h"
#include "value_iterator.h"

namespace esphome {
namespace efs {

class Object {
 public:
  using value_type = ValueIterator::value_type;
  using const_iterator = ValueIterator;

  Object(ObisCode obis_code, uint8_t num_values, std::string_view data)
      : obis_code_{obis_code}, num_values_{num_values}, data_{data} {}
  Object(Object &&other) = default;
  Object &operator=(Object &&other) = default;

  const ObisCode &obis_code() const { return obis_code_; }
  uint8_t num_values() const { return num_values_; }
  const std::string_view &data() const { return data_; }

  const_iterator begin() const { return const_iterator(data_.data(), data_.size(), num_values_); }

  const_iterator end() const { return const_iterator(); }

 private:
  ObisCode obis_code_;
  uint8_t num_values_;
  std::string_view data_;
};

}  // namespace efs
}  // namespace esphome
