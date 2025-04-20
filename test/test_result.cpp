#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::FieldsAre;
using ::testing::Property;
using ::testing::StrEq;

#include "components/efs/obis_code.h"
#include "components/efs/result.h"
#include "components/efs/status.h"

using namespace esphome::efs;

MATCHER_P(ValueEq, value, "") {
  return ExplainMatchResult(FieldsAre(StrEq(value), strlen(value)), arg, result_listener);
}

class ResultTest : public ::testing::Test {
 protected:
};
TEST_F(ResultTest, NormalOperation) {
  const char buffer[] = "Test\0\x02"
                        "\x01\x02\x03\x04\x05\x01\x14\x00Test again\0\0"
                        "\x07\x08\x09\x0A\x0B\x02\x14\x00qwerty\0uiop\0";
  size_t buffer_size = sizeof(buffer);

  const auto result = Result(Status::Ok, buffer, buffer_size);

  EXPECT_THAT(result,
              ElementsAre(AllOf(Property(&Object::obis_code, ObisCode(0, 0, 0, 0, 0)), Property(&Object::num_values, 1),
                                ElementsAre(ValueEq("Test"))),
                          AllOf(Property(&Object::obis_code, ObisCode(1, 2, 3, 4, 5)), Property(&Object::num_values, 1),
                                ElementsAre(ValueEq("Test again"))),
                          AllOf(Property(&Object::obis_code, ObisCode(7, 8, 9, 10, 11)),
                                Property(&Object::num_values, 2), ElementsAre(ValueEq("qwerty"), ValueEq("uiop")))));
}

TEST_F(ResultTest, EmptyBuffer) {
  const char *buffer = "";
  size_t buffer_size = 0;

  const auto result = Result(Status::Ok, buffer, buffer_size);

  EXPECT_EQ(result.begin(), result.end());
}

TEST_F(ResultTest, NullptrBuffer) {
  const char *buffer = "";
  size_t buffer_size = 0;

  const auto result = Result(Status::Ok, buffer, buffer_size);

  EXPECT_EQ(result.begin(), result.end());
}

TEST_F(ResultTest, InsufficientHeaderData) {
  const char buffer[] = "Test\0\x02"
                        "\x07\x08\x09\x0A\x0B\x01\x14";
  size_t buffer_size = sizeof(buffer);

  const auto result = Result(Status::Ok, buffer, buffer_size);

  EXPECT_THAT(result, ElementsAre(AllOf(Property(&Object::obis_code, ObisCode(0, 0, 0, 0, 0)),
                                        Property(&Object::num_values, 1), ElementsAre(ValueEq("Test")))));
}

TEST_F(ResultTest, InvalidObjectSize) {
  const char buffer[] = "Test\0\x01"
                        "\x01\x02\x03\x04\x05\x01\x13\x00XXXXXXXXX\0";
  size_t buffer_size = sizeof(buffer) - 1;

  const auto result = Result(Status::Ok, buffer, buffer_size);

  EXPECT_THAT(result, ElementsAre(AllOf(Property(&Object::obis_code, ObisCode(0, 0, 0, 0, 0)),
                                        Property(&Object::num_values, 1), ElementsAre(ValueEq("Test")))));
}
