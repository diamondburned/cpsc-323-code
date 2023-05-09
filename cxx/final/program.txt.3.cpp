#include <iostream>

int main() {
  int p1, p2q, pr;
  p1 = 3;
  p2q = 4;
  pr = p1 + p2q;
  std::cout << pr << std::endl;
  pr = p1 * (p2q + 2 * pr);
  std::cout << "value=" << pr << std::endl;
  return 0;
}
