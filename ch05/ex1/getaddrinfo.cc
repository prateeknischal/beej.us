#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>
#include <vector>

typedef std::pair<std::string, std::string> __ipstr;

std::pair<std::vector<__ipstr>, int> resolve_hostname(char *hostname) {
  struct addrinfo hints;
  struct addrinfo *servinfo;
  std::vector<std::pair<std::string, std::string>> res;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int status = getaddrinfo(hostname, "443", &hints, &servinfo);
  if (status != 0) {
    std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
    return std::make_pair(res, 1);
  }

  for (struct addrinfo *p = servinfo; p != nullptr; p = p->ai_next) {
    void *addr;
    std::string ipver;

    if (p->ai_family == AF_INET) {
      // IPv4 address
      struct sockaddr_in *ipv4 =
          reinterpret_cast<struct sockaddr_in *>(p->ai_addr);

      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    } else {
      struct sockaddr_in6 *ipv6 =
          reinterpret_cast<struct sockaddr_in6 *>(p->ai_addr);

      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }

    char ipstr[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    res.push_back(make_pair(ipstr, ipver));
  }

  freeaddrinfo(servinfo);
  return make_pair(res, 0);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s hostname [hostname...]\n", argv[0]);
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    auto res = resolve_hostname(argv[i]);
    if (res.second != 0) {
      std::cout << "failed to resolve hostname " << argv[i] << "\n";
      continue;
    }

    std::cout << argv[i] << "\n";
    for (auto v : res.first) {
      std::cout << "\t" << v.second << ": " << v.first << "\n";
    }
  }

  return 0;
}
