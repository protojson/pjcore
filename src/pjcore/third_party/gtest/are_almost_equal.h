#ifndef PJCORE_ARE_ALMOST_EQUAL_H_
#define PJCORE_ARE_ALMOST_EQUAL_H_

namespace pjcore {

bool AreAlmostEqual(const float& left, const float& right);

bool AreAlmostEqual(const double& left, const double& right);

template <typename Value>
bool AreAlmostEqual(Value left, Value right) {
  return left == right;
}

}  // namespace pjcore

#endif  // PJCORE_ARE_ALMOST_EQUAL_H_
