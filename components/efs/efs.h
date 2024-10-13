#pragma once

#ifdef USE_ARDUINO

#include "obis_code.h"
#include "parser.h"
#include "reader.h"

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "esphome/core/defines.h"

#include <map>
#include <vector>

namespace esphome {
namespace efs {

class Efs : public Component, public uart::UARTDevice {
 public:
  Efs(uart::UARTComponent *uart) : uart::UARTDevice(uart) {}

  void setup() override;
  void loop() override;

  bool parse_telegram();

  void dump_config() override;

  void set_decryption_key(const std::string &decryption_key);
  void set_max_telegram_length(size_t length) { this->max_telegram_len_ = length; }
  void set_request_pin(GPIOPin *request_pin) { this->request_pin_ = request_pin; }
  void set_request_interval(uint32_t interval) { this->request_interval_ = interval; }
  void set_receive_timeout(uint32_t timeout) { this->receive_timeout_ = timeout; }
  void add_sensor(const ObisCode &obis_code, sensor::Sensor *sensor) { this->sensors_.emplace(obis_code, sensor); }

 protected:
  void receive_telegram_();
  void receive_encrypted_telegram_();
  void reset_telegram_();

  /// Wait for UART data to become available within the read timeout.
  ///
  /// The smart meter might provide data in chunks, causing available() to
  /// return 0. When we're already reading a telegram, then we don't return
  /// right away (to handle further data in an upcoming loop) but wait a
  /// little while using this method to see if more data are incoming.
  /// By not returning, we prevent other components from taking so much
  /// time that the UART RX buffer overflows and bytes of the telegram get
  /// lost in the process.
  bool available_within_timeout_();

  // Request telegram
  uint32_t request_interval_;
  bool request_interval_reached_();
  GPIOPin *request_pin_{nullptr};
  uint32_t last_request_time_{0};
  bool requesting_data_{false};
  bool ready_to_request_data_();
  void start_requesting_data_();
  void stop_requesting_data_();

  // Read telegram
  uint32_t receive_timeout_;
  bool receive_timeout_reached_();
  size_t max_telegram_len_;
  char *telegram_{nullptr};
  size_t bytes_read_{0};
  uint8_t *crypt_telegram_{nullptr};
  size_t crypt_telegram_len_{0};
  size_t crypt_bytes_read_{0};
  uint32_t last_read_time_{0};
  bool header_found_{false};
  bool footer_found_{false};

  Parser parser_;
  Reader reader_;

  std::map<ObisCode, sensor::Sensor *> sensors_{};
  std::vector<uint8_t> decryption_key_{};
};
}  // namespace efs
}  // namespace esphome

#endif  // USE_ARDUINO
