#ifdef USE_ARDUINO

#include "efs.h"
#include "esphome/core/log.h"

namespace esphome {
namespace efs {

static const char *const TAG = "efs";

void Efs::setup() {
    this->telegram_ = new char[this->max_telegram_len_];  // NOLINT
}

void Efs::loop() {
    if (this->ready_to_request_data_()) {
        this->receive_telegram_();
    }
}

bool Efs::ready_to_request_data_() {
    this->start_requesting_data_();
    if (!this->requesting_data_) {
        while (this->available()) {
            this->read();
            ++this->read_counter_;
        }
    }
    return this->requesting_data_;
}

bool Efs::receive_timeout_reached_() { return millis() - this->last_read_time_ > this->receive_timeout_; }

bool Efs::available_within_timeout_() {
    if (this->available()) {
        this->last_read_time_ = millis();
        return true;
    }
    if (!header_found_) {
        return false;
    }
    if (this->parent_->get_rx_buffer_size() < this->max_telegram_len_) {
        while (!this->receive_timeout_reached_()) {
            delay(5);
            if (this->available()) {
                this->last_read_time_ = millis();
                return true;
            }
        }
    }
    if (this->receive_timeout_reached_()) {
        ESP_LOGW(TAG, "Timeout while reading telegram");
        this->reset_telegram_();
    }

    return false;
}

void Efs::start_requesting_data_() {
    if (!this->requesting_data_) {
        ESP_LOGV(TAG, "Start reading data from P1 port");
        this->requesting_data_ = true;
    }
}

void Efs::stop_requesting_data_() {
    if (this->requesting_data_) {
        ESP_LOGV(TAG, "Stop reading data from P1 port");
        while (this->available()) {
            this->read();
            ++this->read_counter_;
        }
        this->requesting_data_ = false;
    }
}

void Efs::reset_telegram_() {
    this->header_found_ = false;
    this->footer_found_ = false;
    this->bytes_read_ = 0;
    this->last_read_time_ = 0;
}

void Efs::receive_telegram_() {
    while (this->available_within_timeout_()) {
        const char c = this->read();
        ++this->read_counter_;
        if (c == '~') {
            ESP_LOGV(TAG, "Header of telegram found");
            this->reset_telegram_();
            this->header_found_ = true;
            ++this->telegram_counter_;
        }

        if (!this->header_found_)
            continue;

        if (this->bytes_read_ >= this->max_telegram_len_) {
            this->reset_telegram_();
            ESP_LOGE(TAG, "Error: telegram larger than buffer (%d bytes)", this->max_telegram_len_);
            return;
        }

        this->telegram_[this->bytes_read_++] = c;

        if (c == '~') {
            ESP_LOGV(TAG, "Footer of telegram found");
            this->footer_found_ = true;
            continue;
        }
        if (this->footer_found_ && c == '\n') {
            this->parse_telegram();
            this->reset_telegram_();
            return;
        }
    }
}

void Efs::parse_telegram() {
    ESP_LOGV(TAG, "Trying to parse telegram");
    this->stop_requesting_data_();
}

void Efs::dump_config() {
    ESP_LOGCONFIG(TAG, "EFS:");
    ESP_LOGCONFIG(TAG, "  Max telegram length: %d", this->max_telegram_len_);
    ESP_LOGCONFIG(TAG, "  Receive timeout: %.1fs", this->receive_timeout_ / 1e3f);
    ESP_LOGV(TAG, "  Telegrams read: %u", this->telegram_counter_);
    ESP_LOGV(TAG, "  Bytes read: %u", this->bytes_read_);
}

}  // namespace efs
}  // namespace esphome

#endif // USE_ARDUINO
