#include <gtest/gtest.h>
#include <string>
#include <cstring>

#include "components/efs/header.h"
#include "components/efs/parser.h"

using namespace esphome::efs;

class StubCrcCalculator {
 public:
  void update(const char &) {}
  void reset() {}
  uint16_t crc() { return 0; }
};

class ParserTest : public ::testing::Test {
 protected:
  void SetUp() override { memset(buffer, 0, sizeof(buffer)); }

  void LoadBuffer(const std::string &input) {
    ASSERT_LT(input.size(), sizeof(buffer));
    strcpy(buffer, input.c_str());
  }

  BaseParser<StubCrcCalculator> parser;
  char buffer[1024];
};

TEST_F(ParserTest, EmptyBuffer) {
  LoadBuffer("");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::StartNotFound);
}

TEST_F(ParserTest, InvalidStart) {
  LoadBuffer("X123");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::StartNotFound);
}

TEST_F(ParserTest, ValidStart) {
  LoadBuffer("/XYZ\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, ValidCompleteObisCode) {
  LoadBuffer("/XYZ\r\n1-0:1.8.0*255(0)\r\n!0000");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, ValidTruncatedObisCode) {
  LoadBuffer("/XYZ\r\n1-0:1.8.0(0)\r\n!0000");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, InvalidObisCode) {
  LoadBuffer("/XYZ\r\n1-0:999.8.0(0)\r\n!0000");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::InvalidObisCode);
}

TEST_F(ParserTest, CompleteValidTelegram) {
  // Example of a valid telegram with identifier, OBIS code, value, and CRC
  LoadBuffer("/ISK5\r\n1-0:1.8.0*255(123456.78)\r\n!0000\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, MultipleObjects) {
  LoadBuffer("/ISK5\r\n"
             "1-0:1.8.0(123456.78)\r\n"
             "1-0:2.8.0(987654.32)\r\n"
             "!0000\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, InvalidCrcCharactersReturnsInvalidCrc) {
  LoadBuffer("/ISK5\r\n1-0:1.8.0(123)\r\n!XXXX\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::InvalidCrc);
}

TEST_F(ParserTest, IncompleteCrcValueReturnsInvalidCrc) {
  LoadBuffer("/ISK5\r\n1-0:1.8.0(123)\r\n!\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::InvalidCrc);
}

TEST_F(ParserTest, MissingCrcWithoutMarkerIsIgnored) {
  LoadBuffer("/ISK5\r\n1-0:1.8.0*255(123)\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, IncorrectCrcReturnsCrcCheckFailed) {
  LoadBuffer("/ISK5\r\n1-0:1.8.0(123)\r\n!1234\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::CrcCheckFailed);
}

TEST_F(ParserTest, WriteOverflowIsCaught) {
  LoadBuffer("/\n1-1:2.3.4\n");
  auto status = parser.parse_telegram(buffer, sizeof(buffer));
  EXPECT_EQ(status, Status::WriteOverflow);
}

TEST_F(ParserTest, Supports255ByteObjects) {
  // 8B Header, 246 B object + trailing NULL == 255 B
  std::string input = "/ISK5\r\n1-0:1.8.0(";
  for (int i = 0; i < 31; i++) {
    input += "01234567";
  }
  input.pop_back();
  input.pop_back();
  input += ")\r\n!0000\r\n";

  LoadBuffer(input);
  auto status = parser.parse_telegram(buffer, sizeof(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, ReturnsObjectTooLongIfObjectExceeds255B) {
  std::string input = "/ISK5\r\n1-0:1.8.0(";
  for (int i = 0; i < 32; i++) {
    input += "01234567";
  }
  input.pop_back();
  input += ")\r\n!";

  LoadBuffer(input);
  auto status = parser.parse_telegram(buffer, sizeof(buffer));
  EXPECT_EQ(status, Status::ObjectTooLong);
}

TEST_F(ParserTest, HandlesMultipleValuesForObject) {
  LoadBuffer("/ISK5\r\n1-0:1.8.0*255(123)(456)(789)\r\n!0000\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);
}

TEST_F(ParserTest, ParsedDataLayout) {
  LoadBuffer("/ISK5\r\n"
             "1-0:1.8.0*255(123456.78)\r\n"
             "1-0:2.8.0*255(987654.32)(123)\r\n"
             "1-0:3.8.0()(ABC)()\r\n"
             "1-0:4.8.0\r\n"
             "1-0:5.8.0(3.1415)\r\n"
             "!0000\r\n");
  auto status = parser.parse_telegram(buffer, strlen(buffer));
  EXPECT_EQ(status, Status::Ok);

  // Check the identifier string (without the /)
  EXPECT_STREQ(buffer, "ISK5");

  // Get number of objects
  uint8_t *num_objects_ptr = reinterpret_cast<uint8_t *>(buffer + strlen(buffer) + 1);
  EXPECT_EQ(*num_objects_ptr, 5);

  // First header + value
  char *current_pos = reinterpret_cast<char *>(num_objects_ptr + 1);
  Header *header1 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header1->obis_code.value[0], 1);
  EXPECT_EQ(header1->obis_code.value[1], 0);
  EXPECT_EQ(header1->obis_code.value[2], 1);
  EXPECT_EQ(header1->obis_code.value[3], 8);
  EXPECT_EQ(header1->obis_code.value[4], 0);
  EXPECT_EQ(header1->obis_code.value[5], 255);
  EXPECT_EQ(header1->num_values, 1);
  EXPECT_EQ(header1->object_size, sizeof(Header) + 10);
  char *value1 = current_pos + sizeof(Header);
  EXPECT_STREQ(value1, "123456.78");

  // Second header + values
  current_pos += header1->object_size;
  Header *header2 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header2->obis_code.value[0], 1);
  EXPECT_EQ(header2->obis_code.value[1], 0);
  EXPECT_EQ(header2->obis_code.value[2], 2);
  EXPECT_EQ(header2->obis_code.value[3], 8);
  EXPECT_EQ(header2->obis_code.value[4], 0);
  EXPECT_EQ(header2->obis_code.value[5], 255);
  EXPECT_EQ(header2->num_values, 2);
  EXPECT_EQ(header2->object_size, sizeof(Header) + 14);
  char *value2_1 = current_pos + sizeof(Header);
  EXPECT_STREQ(value2_1, "987654.32");
  char *value2_2 = value2_1 + strlen(value2_1) + 1;
  EXPECT_STREQ(value2_2, "123");

  // Third header + values
  current_pos += header2->object_size;
  Header *header3 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header3->obis_code.value[0], 1);
  EXPECT_EQ(header3->obis_code.value[1], 0);
  EXPECT_EQ(header3->obis_code.value[2], 3);
  EXPECT_EQ(header3->obis_code.value[3], 8);
  EXPECT_EQ(header3->obis_code.value[4], 0);
  EXPECT_EQ(header3->obis_code.value[5], 255);
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
  EXPECT_EQ(header4->obis_code.value[0], 1);
  EXPECT_EQ(header4->obis_code.value[1], 0);
  EXPECT_EQ(header4->obis_code.value[2], 4);
  EXPECT_EQ(header4->obis_code.value[3], 8);
  EXPECT_EQ(header4->obis_code.value[4], 0);
  EXPECT_EQ(header4->obis_code.value[5], 255);
  EXPECT_EQ(header4->num_values, 0);
  EXPECT_EQ(header4->object_size, sizeof(Header));

  // Fifth header + value
  current_pos += header4->object_size;
  Header *header5 = reinterpret_cast<Header *>(current_pos);
  EXPECT_EQ(header5->obis_code.value[0], 1);
  EXPECT_EQ(header5->obis_code.value[1], 0);
  EXPECT_EQ(header5->obis_code.value[2], 5);
  EXPECT_EQ(header5->obis_code.value[3], 8);
  EXPECT_EQ(header5->obis_code.value[4], 0);
  EXPECT_EQ(header5->obis_code.value[5], 255);
  EXPECT_EQ(header5->num_values, 1);
  EXPECT_EQ(header5->object_size, sizeof(Header) + 7);
  char *value5_1 = current_pos + sizeof(Header);
  EXPECT_STREQ(value5_1, "3.1415");
}
