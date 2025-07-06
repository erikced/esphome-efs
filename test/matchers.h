#pragma once
#include <gmock/gmock.h>

#include "components/efs/object.h"

namespace esphome::efs::testing {

using ::testing::AllOf;
using ::testing::FieldsAre;
using ::testing::Property;
using ::testing::StrEq;

// Keep custom matcher names consistent with GTest matchers. NOLINTBEGIN(readability-identifier-naming)
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
// NOLINTEND(readability-identifier-naming)

}  // namespace esphome::efs::testing
