#include <type_traits>
#include <google/protobuf/stubs/type_traits.h>

namespace google {
namespace protobuf {
namespace internal {
template <typename From, typename To>
struct is_convertible
	: integral_constant<bool, std::is_convertible<From, To>::value > {};
using std::is_enum;
}  // namespace internal
}  // namespace protobuf
}  // namespace google
