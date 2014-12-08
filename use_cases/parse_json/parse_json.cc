#include <iostream>

#include "pjcore/json.h"
#include "pjcore/error_util.h"

#include "article.pb.h"


int main(int, const char**) {
  pjcore::Error error;
  pjcore::JsonValue json;

  if (!pjcore::ReadJson("{\"title\":\"Lorem Ipsum\","
                          "\"content\":\"Dolor sit amet.\"}",
                        &json, &error)) {
    std::cerr << "Error: " << pjcore::ErrorToString(error) << std::endl;
  } else {
    Article article;
    if (! pjcore::UnboxJsonValue(json, &article, &error)) {
      std::cerr << "Error: " << pjcore::ErrorToString(error) << std::endl;
    } else {
      std::cout << article.DebugString() << std::endl;
    }
  }

  return 0;
}
