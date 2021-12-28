#include <iostream>
#include <unistd.h>

int main() {
  char h[64];
  if (gethostname(h, sizeof h) != 0) {
    std::cerr << "failed to get hostname " << errno << std::endl;
    return errno;
  }

  std::cout << h << std::endl;
  return 0;
}
