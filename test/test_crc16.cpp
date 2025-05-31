#include <gtest/gtest.h>
#include <string>

#include "components/efs/crc16.h"

namespace esphome::efs {
namespace {

// Test calculation of CRC-16/ARC values
class Crc16CalculatorTest : public ::testing::Test {
 protected:
  void SetUp() override { calc_.reset(); }

  Crc16Calculator calc_;
};

TEST_F(Crc16CalculatorTest, InitialState) { EXPECT_EQ(calc_.crc(), 0); }

TEST_F(Crc16CalculatorTest, SingleChar) {
  calc_.update('A');
  EXPECT_EQ(calc_.crc(), 0x30C0);
}

TEST_F(Crc16CalculatorTest, Reset) {
  calc_.update('A');
  EXPECT_NE(calc_.crc(), 0);

  calc_.reset();
  EXPECT_EQ(calc_.crc(), 0);

  calc_.update('A');
  EXPECT_EQ(calc_.crc(), 0x30C0);
}

TEST_F(Crc16CalculatorTest, EmptyString) {
  std::string empty_str = "";
  for (char c : empty_str) {
    calc_.update(c);
  }
  EXPECT_EQ(calc_.crc(), 0);
}

TEST_F(Crc16CalculatorTest, MultipleChars) {
  std::string test_str = "Hello, World!";
  for (char c : test_str) {
    calc_.update(c);
  }
  EXPECT_EQ(calc_.crc(), 0xFA4D);
}

}  // namespace
}  // namespace esphome::efs
