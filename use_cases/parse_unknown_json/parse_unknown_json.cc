#include <iostream>

#include "pjcore/json.h"
#include "pjcore/error_util.h"

#include "review.pb.h"

int main(int, const char**) {
  pjcore::Error error;
  pjcore::JsonValue json;

  if (!pjcore::ReadJson("{\"reviewer\":\"Vegeta\","
                          "\"rating\":9001,"
                          "\"summary\":\"It's over nine thousand!\"}",
                        &json, &error)) {
    std::cerr << "Error: " << pjcore::ErrorToString(error) << std::endl;
  } else {
    Review review;
    if (!pjcore::UnboxJsonValue(json, &review, &error)) {
      std::cerr << "Error: " << pjcore::ErrorToString(error) << std::endl;
    } else {
      std::cout << review.DebugString() << std::endl;
    }
  }

  return 0;
}

