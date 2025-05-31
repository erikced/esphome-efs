#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "components/efs/header.h"
#include "components/efs/parser.h"

using std::literals::operator""sv;

namespace esphome::efs {
namespace {

class StubCrcCalculator {
 public:
  void update(const char &) {}
  void reset() {}
  uint16_t crc() { return 0; }
};

class ParserTest : public ::testing::Test {
 protected:
  void SetUp() override { buffer_.clear(); }

  void load_buffer_(const std::string_view &input) {
    buffer_.reserve(input.size());
    std::copy(input.begin(), input.end(), std::back_inserter(buffer_));
  }

  BaseParser<StubCrcCalculator> parser_;
  std::vector<char> buffer_;
};

TEST_F(ParserTest, EmptyBuffer) {
  load_buffer_(""sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::START_NOT_FOUND);
}

TEST_F(ParserTest, InvalidStart) {
  load_buffer_("X123"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::START_NOT_FOUND);
}

TEST_F(ParserTest, ValidStart) {
  load_buffer_("/XYZ\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, ValidTruncatedObisCode) {
  load_buffer_("/XYZ\r\n1-0:1.8.0(0)\r\n!0000"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, ValidCompleteObisCode) {
  load_buffer_("/XYZ\r\n1-0:1.8.0*255(0)\r\n!0000"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, ValidCompleteObisCodeNotEndingWith255IsNotSupported) {
  load_buffer_("/XYZ\r\n1-0:1.8.0*254(0)\r\n!0000"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::INVALID_OBIS_CODE);
}

TEST_F(ParserTest, InvalidObisCode) {
  load_buffer_("/XYZ\r\n1-0:999.8.0(0)\r\n!0000"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::INVALID_OBIS_CODE);
}

TEST_F(ParserTest, CompleteValidTelegram) {
  load_buffer_("/ISK5\r\n1-0:1.8.0*255(123456.78)\r\n!0000\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, MultipleObjects) {
  load_buffer_("/ISK5\r\n"
               "1-0:1.8.0(123456.78)\r\n"
               "1-0:2.8.0(987654.32)\r\n"
               "!0000\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, InvalidCrcCharactersReturnsInvalidCrc) {
  load_buffer_("/ISK5\r\n1-0:1.8.0(123)\r\n!XXXX\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::INVALID_CRC);
}

TEST_F(ParserTest, IncompleteCrcValueReturnsInvalidCrc) {
  load_buffer_("/ISK5\r\n1-0:1.8.0(123)\r\n!\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::INVALID_CRC);
}

TEST_F(ParserTest, MissingCrcWithoutMarkerIsIgnored) {
  load_buffer_("/ISK5\r\n1-0:1.8.0*255(123)\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, IncorrectCrcReturnsCrcCheckFailed) {
  load_buffer_("/ISK5\r\n1-0:1.8.0(123)\r\n!1234\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::CRC_CHECK_FAILED);
}

TEST_F(ParserTest, Supports8kBObjects) {
  // 8B Header, 8182B value + NULL terminator + padding (NULL) = 8192B
  std::string input = "/\r\n1-0:1.8.0(";
  for (int i = 0; i < 818; i++) {
    input += "0123456789";
  }
  input += "01";
  input += ")\r\n";

  load_buffer_(input);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, ReturnsObjectTooLongIfObjectExceeds8kB) {
  std::string input = "/ISK5\r\n1-0:1.8.0(";
  for (int i = 0; i < 818; i++) {
    input += "0123456789";
  }
  input += "0123";
  input += ")\r\n!1234";

  load_buffer_(input);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OBJECT_TOO_LONG);
}

TEST_F(ParserTest, HandlesMultipleValuesForObject) {
  load_buffer_("/ISK5\r\n1-0:1.8.0*255(123)(456)(789)\r\n!0000\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);
}

TEST_F(ParserTest, ParsedDataLayout) {
  load_buffer_("/ISK5\r\n"
               "1-0:1.8.0*255(123456.78)\r\n"
               "1-0:2.8.0*255(987654.32)(123)\r\n"
               "1-0:3.8.0()(ABC)()\r\n"
               "1-0:4.8.0\r\n"
               "1-0:5.8.0(3.1415)\r\n"
               "!0000\r\n"sv);
  auto result = parser_.parse_telegram(buffer_.data(), buffer_.size());
  EXPECT_EQ(result.status, Status::OK);

  // Check the identifier string (without the /)
  EXPECT_STREQ(buffer_.data(), "ISK5");

  // Get number of objects
  uint8_t *num_objects_ptr = reinterpret_cast<uint8_t *>(buffer_.data() + strlen(buffer_.data()) + 1);
  EXPECT_EQ(*num_objects_ptr, 5);

  // First header + value
  char *current_pos = reinterpret_cast<char *>(num_objects_ptr + 1);
  Header *header1 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header1->obis_code[0], 1);
  EXPECT_EQ(header1->obis_code[1], 0);
  EXPECT_EQ(header1->obis_code[2], 1);
  EXPECT_EQ(header1->obis_code[3], 8);
  EXPECT_EQ(header1->obis_code[4], 0);
  EXPECT_EQ(header1->num_values, 1);
  EXPECT_EQ(header1->object_size, sizeof(Header) + 10);
  char *value1 = current_pos + sizeof(Header);
  EXPECT_STREQ(value1, "123456.78");

  // Second header + values
  current_pos += header1->object_size;
  Header *header2 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header2->obis_code[0], 1);
  EXPECT_EQ(header2->obis_code[1], 0);
  EXPECT_EQ(header2->obis_code[2], 2);
  EXPECT_EQ(header2->obis_code[3], 8);
  EXPECT_EQ(header2->obis_code[4], 0);
  EXPECT_EQ(header2->num_values, 2);
  EXPECT_EQ(header2->object_size, sizeof(Header) + 14);
  char *value2_1 = current_pos + sizeof(Header);
  EXPECT_STREQ(value2_1, "987654.32");
  char *value2_2 = value2_1 + strlen(value2_1) + 1;
  EXPECT_STREQ(value2_2, "123");

  // Third header + values
  current_pos += header2->object_size;
  Header *header3 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header3->obis_code[0], 1);
  EXPECT_EQ(header3->obis_code[1], 0);
  EXPECT_EQ(header3->obis_code[2], 3);
  EXPECT_EQ(header3->obis_code[3], 8);
  EXPECT_EQ(header3->obis_code[4], 0);
  EXPECT_EQ(header3->num_values, 3);
  EXPECT_EQ(header3->object_size, sizeof(Header) + 6);
  char *value3_1 = current_pos + sizeof(Header);
  EXPECT_STREQ(value3_1, "");
  char *value3_2 = value3_1 + strlen(value3_1) + 1;
  EXPECT_STREQ(value3_2, "ABC");
  char *value3_3 = value3_1 + strlen(value3_2) + 1;
  EXPECT_STREQ(value3_3, "");

  // Fourth header (without value)
  current_pos += header3->object_size;
  Header *header4 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header4->obis_code[0], 1);
  EXPECT_EQ(header4->obis_code[1], 0);
  EXPECT_EQ(header4->obis_code[2], 4);
  EXPECT_EQ(header4->obis_code[3], 8);
  EXPECT_EQ(header4->obis_code[4], 0);
  EXPECT_EQ(header4->num_values, 0);
  EXPECT_EQ(header4->object_size, sizeof(Header));

  // Fifth header + value
  current_pos += header4->object_size;
  Header *header5 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header5->obis_code[0], 1);
  EXPECT_EQ(header5->obis_code[1], 0);
  EXPECT_EQ(header5->obis_code[2], 5);
  EXPECT_EQ(header5->obis_code[3], 8);
  EXPECT_EQ(header5->obis_code[4], 0);
  EXPECT_EQ(header5->num_values, 1);
  EXPECT_EQ(header5->object_size, sizeof(Header) + 8);
  char *value5_1 = current_pos + sizeof(Header);
  EXPECT_STREQ(value5_1, "3.1415");
}

}  // namespace
}  // namespace esphome::efs
