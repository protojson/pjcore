#include <iostream>

#include "business.pb.h"
#include "pjcore/json.h"

int main(int, const char**) {
  Business bookstore;
  bookstore.set_name("The Shop Around The Corner");
  bookstore.set_url("http://bookstore.com");

  std::cout << pjcore::WriteJson(bookstore) << std::endl;
  return 0;
}
