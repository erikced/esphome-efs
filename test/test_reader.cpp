#include <gtest/gtest.h>

#include "components/efs/obis_code.h"
#include "components/efs/reader.h"

using namespace esphome::efs;

class ReaderTest : public ::testing::Test {
 protected:
  Reader reader;
};

TEST_F(ReaderTest, NormalOperation) {
  const char buffer[] = "Test\0\x02"
                        "\x01\x02\x03\x04\x05\x01\x13\x00Test again\0"
                        "\x07\x08\x09\x0A\x0B\x02\x14\x00qwerty\0uiop\0";
  size_t buffer_size = sizeof(buffer);

  int call_count = 0;
  reader.read_parsed_data(buffer, buffer_size, [&](const ObisCode &obis, uint8_t num_values, const char *data) {
    ++call_count;
    if (call_count == 1) {
      EXPECT_EQ(obis, ObisCode(0, 0, 0, 0, 0));
      EXPECT_EQ(num_values, 1);
      EXPECT_STREQ(data, "Test");
    } else if (call_count == 2) {
      EXPECT_EQ(obis, ObisCode(1, 2, 3, 4, 5));
      EXPECT_EQ(num_values, 1);
      EXPECT_STREQ(data, "Test again");
    } else if (call_count == 3) {
      EXPECT_EQ(obis, ObisCode(7, 8, 9, 10, 11));
      EXPECT_EQ(num_values, 2);
      EXPECT_STREQ(data, "qwerty");
      EXPECT_STREQ(data + 7, "uiop");
    }
  });
  EXPECT_EQ(call_count, 3);
}

TEST_F(ReaderTest, EmptyBuffer) {
  const char *buffer = "";
  size_t buffer_size = 0;

  int call_count = 0;
  reader.read_parsed_data(buffer, buffer_size, [&](const ObisCode &obis_code, uint8_t num_values, const char *data) {
    ++call_count;
    EXPECT_EQ(obis_code, ObisCode(0, 0, 0, 0, 0));
    EXPECT_EQ(num_values, 1);
    EXPECT_STREQ(data, "");
  });
  EXPECT_EQ(call_count, 1);
}

TEST_F(ReaderTest, NullptrBuffer) {
  const char *buffer = nullptr;
  size_t buffer_size = 0;

  int call_count = 0;
  reader.read_parsed_data(buffer, buffer_size, [&](const ObisCode &, uint8_t, const char *) { ++call_count; });
  EXPECT_EQ(call_count, 0);
}

TEST_F(ReaderTest, InsufficientHeaderData) {
  const char buffer[] = "Test\0\x02"
                        "\x07\x08\x09\x0A\x0B\x0C\x01";
  size_t buffer_size = sizeof(buffer) - 1;

  int call_count = 0;
  reader.read_parsed_data(buffer, buffer_size, [&](const ObisCode &, uint8_t, const char *) { ++call_count; });
  EXPECT_EQ(call_count, 1);
}

TEST_F(ReaderTest, InvalidObjectSize) {
  const char buffer[] = "Test\0\x01"
                        "\x01\x02\x03\x04\x05\x01\x13\x00XXXXXXXXX\0";
  size_t buffer_size = sizeof(buffer) - 1;

  int call_count = 0;
  reader.read_parsed_data(buffer, buffer_size, [&](const ObisCode &, uint8_t, const char *) { ++call_count; });
  EXPECT_EQ(call_count, 1);
}
