#include <array>
#include <vector>
#include <tuple>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "components/efs/crc16.h"
#include "components/efs/header.h"
#include "components/efs/obis_code.h"
#include "components/efs/object.h"
#include "components/efs/parser.h"
#include "components/efs/result.h"

using ::testing::AllOf;
using ::testing::ElementsAreArray;
using ::testing::FieldsAre;
using ::testing::Property;
using ::testing::StrEq;

namespace esphome::efs {
namespace {

MATCHER_P(ValueEq, value, "") {
  return ExplainMatchResult(FieldsAre(StrEq(value), strlen(value)), arg, result_listener);
}

MATCHER_P(ValueArray, values, "") {
  auto ai = arg.begin();
  auto vi = values.begin();

  for (; ai != arg.end() && vi != values.end(); ++ai, ++vi) {
    if (!ExplainMatchResult(ValueEq(*vi), *ai, result_listener)) {
      return false;
    }
  }
  return vi == values.end() && ai == arg.end();
}

MATCHER_P2(ObjectLike, obis_code, values, "") {
  return ExplainMatchResult(
      AllOf(Property(&Object::obis_code, obis_code), Property(&Object::num_values, values.size()), ValueArray(values)),
      arg, result_listener);
}

const char SAMPLE_TELEGRAM[] =
    "/ISk5\\2MT382-1000\r\n"
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

testing::Matcher<Object> match_object(ObisCode obis_code, std::vector<const char *> values) {
  return ObjectLike(obis_code, std::move(values));
}

const std::array<testing::Matcher<Object>, 28> EXPECTED_OUTPUT{
    match_object({0, 0, 0, 0, 0}, {"ISk5\\2MT382-1000"}),
    match_object({1, 3, 0, 2, 8}, {"40"}),
    match_object({0, 0, 1, 0, 0}, {"101209113020W"}),
    match_object({0, 0, 96, 1, 1}, {"4B384547303034303436333935353037"}),
    match_object({1, 0, 1, 8, 1}, {"123456.789*kWh"}),
    match_object({1, 0, 1, 8, 2}, {"123456.789*kWh"}),
    match_object({1, 0, 2, 8, 1}, {"123456.789*kWh"}),
    match_object({1, 0, 2, 8, 2}, {"123456.789*kWh"}),
    match_object({0, 0, 96, 14, 0}, {"0002"}),
    match_object({1, 0, 1, 7, 0}, {"01.193*kW"}),
    match_object({1, 0, 2, 7, 0}, {"00.000*kW"}),
    match_object({0, 0, 17, 0, 0}, {"016.1*kW"}),
    match_object({0, 0, 96, 3, 10}, {"1"}),
    match_object({0, 0, 96, 7, 21}, {"00004"}),
    match_object({0, 0, 96, 7, 9}, {"00002"}),
    match_object({1, 0, 99, 97, 0},
                  {"2", "0:96.7.19", "101208152415W", "0000000240*s", "101208151004W", "00000000301*s"}),
    match_object({1, 0, 32, 32, 0}, {"00002"}),
    match_object({1, 0, 52, 32, 0}, {"00001"}),
    match_object({1, 0, 72, 32, 0}, {"00000"}),
    match_object({1, 0, 32, 36, 0}, {"00000"}),
    match_object({1, 0, 52, 36, 0}, {"00003"}),
    match_object({1, 0, 72, 36, 0}, {"00000"}),
    match_object({0, 0, 96, 13, 1}, {"3031203631203831"}),
    match_object({0, 0, 96, 13, 0}, {"303132333435363738393A3B3C3D3E3F303132333435363738393A3B3C3D3E3F3"
                                      "03132333435363738393A3B3C3D3E3F3031323334353637"
                                      "38393A3B3C3D3E3F303132333435363738393A3B3C3D3E3F"}),
    match_object({0, 1, 24, 1, 0}, {"03"}),
    match_object({0, 1, 96, 1, 0}, {"3232323241424344313233343536373839"}),
    match_object({0, 1, 24, 2, 1}, {"101209110000W", "12785.123*m3"}),
    match_object({0, 1, 24, 4, 0}, {"1"})};

class IntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override { strncpy(buffer_, SAMPLE_TELEGRAM, sizeof(buffer_)); }

  Parser parser_;
  alignas(2) char buffer_[1024];
};

TEST_F(IntegrationTest, TestSampleTelegramParsing) {
  const auto result = parser_.parse_telegram(buffer_, sizeof(buffer_));
  ASSERT_EQ(result.status, Status::OK);

  EXPECT_THAT(result, ElementsAreArray(EXPECTED_OUTPUT));
}

}  // namespace
}  // namespace esphome::efs
