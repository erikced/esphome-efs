#include <vector>
#include <tuple>

#include <gtest/gtest.h>

#include "components/efs/crc16.h"
#include "components/efs/header.h"
#include "components/efs/obis_code.h"
#include "components/efs/parser.h"
#include "components/efs/reader.h"

const char *sample_telegram = "/ISk5\\2MT382-1000\r\n"
                              "\r\n"
                              "1-3:0.2.8(40)\r\n"
                              "0-0:1.0.0(101209113020W)\r\n"
                              "0-0:96.1.1(4B384547303034303436333935353037)\r\n"
                              "1-0:1.8.1(123456.789*kWh)\r\n"
                              "1-0:1.8.2(123456.789*kWh)\r\n"
                              "1-0:2.8.1(123456.789*kWh)\r\n"
                              "1-0:2.8.2(123456.789*kWh)\r\n"
                              "0-0:96.14.0(0002)\r\n"
                              "1-0:1.7.0(01.193*kW)\r\n"
                              "1-0:2.7.0(00.000*kW)\r\n"
                              "0-0:17.0.0(016.1*kW)\r\n"
                              "0-0:96.3.10(1)\r\n"
                              "0-0:96.7.21(00004)\r\n"
                              "0-0:96.7.9(00002)\r\n"
                              "1-0:99:97.0(2)(0:96.7.19)(101208152415W)(0000000240*s)(101208151004W)(00000000301*s)\r\n"
                              "1-0:32.32.0(00002)\r\n"
                              "1-0:52.32.0(00001)\r\n"
                              "1-0:72:32.0(00000)\r\n"
                              "1-0:32.36.0(00000)\r\n"
                              "1-0:52.36.0(00003)\r\n"
                              "1-0:72.36.0(00000)\r\n"
                              "0-0:96.13.1(3031203631203831)\r\n"
                              "0-0:96.13.0("
                              "303132333435363738393A3B3C3D3E3F303132333435363738393A3B3C3D3E3F303132333435363738393A3B"
                              "3C3D3E3F303132333435363738393A3B3C3D3E3F303132333435363738393A3B3C3D3E3F)\r\n"
                              "0-1:24.1.0(03)\r\n"
                              "0-1:96.1.0(3232323241424344313233343536373839)\r\n"
                              "0-1:24.2.1(101209110000W)(12785.123*m3)\r\n"
                              "0-1:24.4.0(1)\r\n"
                              "!F46A\r\n";

using namespace esphome::efs;
using OutputObject = std::tuple<const ObisCode, const std::vector<const char *>>;

const OutputObject expected_output[]{
    {ObisCode{0, 0, 0, 0, 0}, {"ISk5\\2MT382-1000"}},
    {ObisCode{1, 3, 0, 2, 8}, {"40"}},
    {ObisCode{0, 0, 1, 0, 0}, {"101209113020W"}},
    {ObisCode{0, 0, 96, 1, 1}, {"4B384547303034303436333935353037"}},
    {ObisCode{1, 0, 1, 8, 1}, {"123456.789*kWh"}},
    {ObisCode{1, 0, 1, 8, 2}, {"123456.789*kWh"}},
    {ObisCode{1, 0, 2, 8, 1}, {"123456.789*kWh"}},
    {ObisCode{1, 0, 2, 8, 2}, {"123456.789*kWh"}},
    {ObisCode{0, 0, 96, 14, 0}, {"0002"}},
    {ObisCode{1, 0, 1, 7, 0}, {"01.193*kW"}},
    {ObisCode{1, 0, 2, 7, 0}, {"00.000*kW"}},
    {ObisCode{0, 0, 17, 0, 0}, {"016.1*kW"}},
    {ObisCode{0, 0, 96, 3, 10}, {"1"}},
    {ObisCode{0, 0, 96, 7, 21}, {"00004"}},
    {ObisCode{0, 0, 96, 7, 9}, {"00002"}},
    {ObisCode{1, 0, 99, 97, 0}, {"2", "0:96.7.19", "101208152415W", "0000000240*s", "101208151004W", "00000000301*s"}},
    {ObisCode{1, 0, 32, 32, 0}, {"00002"}},
    {ObisCode{1, 0, 52, 32, 0}, {"00001"}},
    {ObisCode{1, 0, 72, 32, 0}, {"00000"}},
    {ObisCode{1, 0, 32, 36, 0}, {"00000"}},
    {ObisCode{1, 0, 52, 36, 0}, {"00003"}},
    {ObisCode{1, 0, 72, 36, 0}, {"00000"}},
    {ObisCode{0, 0, 96, 13, 1}, {"3031203631203831"}},
    {ObisCode{0, 0, 96, 13, 0},
     {"303132333435363738393A3B3C3D3E3F303132333435363738393A3B3C3D3E3F303132333435363738393A3B3C3D3E3F3031323334353637"
      "38393A3B3C3D3E3F303132333435363738393A3B3C3D3E3F"}},
    {ObisCode{0, 1, 24, 1, 0}, {"03"}},
    {ObisCode{0, 1, 96, 1, 0}, {"3232323241424344313233343536373839"}},
    {ObisCode{0, 1, 24, 2, 1}, {"101209110000W", "12785.123*m3"}},
    {ObisCode{0, 1, 24, 4, 0}, {"1"}}};
const size_t num_expected_output_objects = sizeof(expected_output) / sizeof(OutputObject);

class IntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override { strncpy(buffer, sample_telegram, sizeof(buffer)); }

  Parser parser;
  Reader reader;
  char buffer[1024];
};

TEST_F(IntegrationTest, TestSampleTelegramParsing) {
  const auto rv = parser.parse_telegram(buffer, sizeof(buffer));
  ASSERT_EQ(rv, Status::Ok);
  size_t call_count = 0;
  reader.read_parsed_data(buffer, sizeof(buffer), [&](const ObisCode &obis, uint8_t num_values, const char *data) {
    ASSERT_EQ(obis, std::get<0>(expected_output[call_count]));
    const auto &values = std::get<1>(expected_output[call_count]);
    ASSERT_EQ(num_values, values.size());
    EXPECT_EQ(strcmp(data, values[0]), 0);
    ++call_count;
  });
  EXPECT_EQ(call_count, num_expected_output_objects);
}
