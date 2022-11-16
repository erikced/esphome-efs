#pragma once

#ifdef USE_ARDUINO

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "esphome/core/defines.h"

namespace esphome {
namespace efs {


class Efs : public Component, public uart::UARTDevice {
 public:
  Efs(uart::UARTComponent *uart, bool crc_check) : uart::UARTDevice(uart), crc_check_(crc_check) {}

  void setup() override;
  void loop() override;

  void parse_telegram();

  void dump_config() override;

  void set_max_telegram_length(size_t length) { this->max_telegram_len_ = length; }
  void set_receive_timeout(uint32_t timeout) { this->receive_timeout_ = timeout; }

 protected:
  void receive_telegram_();
  void reset_telegram_();
  bool available_within_timeout_();

  bool requesting_data_{false};
  bool ready_to_request_data_();
  void start_requesting_data_();
  void stop_requesting_data_();

  uint32_t receive_timeout_{200};
  bool receive_timeout_reached_();
  size_t max_telegram_len_;
  char *telegram_{nullptr};
  size_t bytes_read_{0};
  uint32_t last_read_time_{0};
  uint32_t telegram_counter_{0};
  uint32_t read_counter_{0};
  bool header_found_{false};
  bool footer_found_{false};
  bool crc_check_;
};
}  // namespace efs
}  // namespace esphome

#endif  // USE_ARDUINO
