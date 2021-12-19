#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#define HTTP_DELIMTER "\r\n"

void usage(int argc, char *argv[]) {
  fprintf(stderr, "Usage: %s hostname [port:80]\n", argv[0]);
}

std::string make_request(std::string path, std::string host) {
  std::string s;
  s += "GET " + path + " HTTP/1.1" + HTTP_DELIMTER;
  s += "Host: " + host + HTTP_DELIMTER;
  s += HTTP_DELIMTER;

  return s;
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    usage(argc, argv);
    return 0;
  }

  char *hostname = argv[1];
  char port[] = "80";
  if (argc == 3) {
    char *port = argv[2];
  }

  struct addrinfo hints = {.ai_flags = AI_PASSIVE,
                           .ai_family = AF_UNSPEC,
                           .ai_socktype = SOCK_STREAM};

  struct addrinfo *remote_addr;

  int status = getaddrinfo(hostname, port, &hints, &remote_addr);
  if (status != 0) {
    std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
    return 1;
  }

  int fd = socket(remote_addr->ai_family, remote_addr->ai_socktype,
                  remote_addr->ai_protocol);
  if (fd == -1) {
    std::cerr << "socket call failed, err: " << errno << "\n";
    return 1;
  }

  status = connect(fd, remote_addr->ai_addr, remote_addr->ai_addrlen);

  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
    std::cerr << "failed to set SO_REUSEADDR to the socket"
              << "\n";
    return 1;
  }

  if (status != 0) {
    std::cerr << "failed to connect, err: " << errno << "\n";
    return 1;
  }

  // Send some message to the server
  std::string msg = make_request("/", hostname);
  int bytes_sent = send(fd, msg.c_str(), msg.length(), 0);
  if (bytes_sent == 0) {
    std::cerr << "failed to send any bytes\n";
    return 1;
  }

  char buf[1024];
  int bytes_read = 1024;
  while (bytes_read == 1024) {
    memset(buf, 0, sizeof(buf));
    bytes_read = recv(fd, buf, sizeof(buf), 0);
    std::cout << buf;
  }

  shutdown(fd, 2);
  freeaddrinfo(remote_addr);
}
