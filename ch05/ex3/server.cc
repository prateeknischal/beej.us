#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define LISTEN_PORT "8000"
#define CONN_LIMIT 10
#define BUFFER_SIZE 1024

// create a server and return the file descriptor.
int create_server() {
  struct addrinfo hint = {
      .ai_flags = AI_PASSIVE,
      .ai_family = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM,
  };

  struct addrinfo *res;
  int ret = getaddrinfo(nullptr, LISTEN_PORT, &hint, &res);
  if (ret != 0) {
    std::cerr << "failed to get address info: " << gai_strerror(ret) << "\n";
    exit(1);
  }

  int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (fd == -1) {
    std::cerr << "failed to create a socket: " << errno << "\n";
    exit(1);
  }

  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0) {
    std::cerr << "failed to set SO_REUSEADDR" << errno << "\n";
    exit(1);
  }

  if ((ret = bind(fd, res->ai_addr, res->ai_addrlen)) != 0) {
    std::cerr << "failed to bind to the socket fd: " << errno << "\n";
    exit(1);
  }

  if ((ret = listen(fd, CONN_LIMIT)) != 0) {
    std::cerr << "failed to listen on the socket: " << errno << "\n";
    exit(1);
  }

  return fd;
  freeaddrinfo(res);
}

// Listen for connections on the server and behave as an echo server. This will
// read the request and reply back the same to the client. The connection will
// be closed when the message starts with "QUIT".
void server_connections(int sockfd) {
  struct sockaddr_storage *remote_addr;
  socklen_t addr_size;
  int fd = accept(sockfd, (struct sockaddr *)remote_addr, &addr_size);
  std::cout << "[" << fd << "] "
            << "got a connection"
            << "\n";

  char buf[BUFFER_SIZE];
  int bytes_read = BUFFER_SIZE;
  while (true) {
    memset(buf, 0, sizeof(buf));
    bytes_read = recv(fd, buf, sizeof(buf), 0);
    if (bytes_read != 0) {
      if (strncmp("QUIT", buf, 4) == 0) {
        std::cout << "[" << fd << "] "
                  << "closing connection for client\n";
        break;
      }
    }
    send(fd, buf, bytes_read, 0);
    std::cout << "[" << fd << "] "
              << "echoing back :" << buf;
  }

  shutdown(fd, 2);
}

int main() {
  int fd = create_server();
  server_connections(fd);
  return 0;
}
