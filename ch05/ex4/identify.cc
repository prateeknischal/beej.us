#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <ostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int create_socket() {
  struct addrinfo hint = {
      .ai_flags = AI_PASSIVE,
      .ai_family = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM,
  };
  struct addrinfo *remote_addr;

  int rc = getaddrinfo("www.example.com", "80", &hint, &remote_addr);
  if (rc != 0) {
    std::cerr << "Failed to get address info " << gai_strerror(rc) << std::endl;
    return rc;
  }

  int fd = socket(remote_addr->ai_family, remote_addr->ai_socktype,
                  remote_addr->ai_protocol);
  if (fd == -1) {
    std::cerr << "Failed to create a socket " << errno << std::endl;
    return errno;
  }

  if (connect(fd, remote_addr->ai_addr, remote_addr->ai_addrlen) != 0) {
    std::cerr << "Failed to connect " << errno << std::endl;
    return errno;
  }

  return fd;
}

int get_peer(int sockfd) {
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  int rc = getpeername(sockfd, (sockaddr *)&addr, &addrlen);
  if (rc != 0) {
    std::cerr << "Failed to get peername " << errno << std::endl;
    return errno;
  }

  char buf[INET6_ADDRSTRLEN];
  inet_ntop(addr.sin_family, &(addr.sin_addr), buf, INET6_ADDRSTRLEN);
  std::cerr << "IP Address : " << std::string(buf) << std::endl;
  std::cerr << "Peer Port  : " << ntohs(addr.sin_port) << std::endl;
  return 0;
}

int main() {
  int sockfd = create_socket();
  get_peer(sockfd);
  shutdown(sockfd, 2);
}
