#include <gtest/gtest.h>
#include <string>

#include "components/efs/crc16.h"

using namespace esphome::efs;

// Test calculation of CRC-16/ARC values
class Crc16CalculatorTest : public ::testing::Test {
 protected:
  void SetUp() override { calc.reset(); }

  Crc16Calculator calc;
};

TEST_F(Crc16CalculatorTest, InitialState) { EXPECT_EQ(calc.crc(), 0); }

TEST_F(Crc16CalculatorTest, SingleChar) {
  calc.update('A');
  EXPECT_EQ(calc.crc(), 0x30C0);
}

TEST_F(Crc16CalculatorTest, Reset) {
  calc.update('A');
  EXPECT_NE(calc.crc(), 0);

  calc.reset();
  EXPECT_EQ(calc.crc(), 0);

  calc.update('A');
  EXPECT_EQ(calc.crc(), 0x30C0);
}

TEST_F(Crc16CalculatorTest, EmptyString) {
  std::string empty_str = "";
  for (char c : empty_str) {
    calc.update(c);
  }
  EXPECT_EQ(calc.crc(), 0);
}

TEST_F(Crc16CalculatorTest, MultipleChars) {
  std::string test_str = "Hello, World!";
  for (char c : test_str) {
    calc.update(c);
  }
  EXPECT_EQ(calc.crc(), 0xFA4D);
}
