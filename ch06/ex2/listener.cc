#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utility>

#define PORT "5000"
#define MAXBUFLEN 128

void *get_in_addr(struct sockaddr *addr) {
  if (addr->sa_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)addr;
    return &s->sin_addr;
  }

  struct sockaddr_in6 *s = (struct sockaddr_in6 *)addr;
  return &s->sin6_addr;
}

std::pair<addrinfo *, int>
create_socket_with_port(const char *host, const char *port, bool to_bind) {
  int sockfd = -1;
  struct addrinfo hints = {
      .ai_flags = AI_PASSIVE,
      .ai_family = AF_INET,
      .ai_socktype = SOCK_DGRAM,
  };

  struct addrinfo *servinfo;
  int rc = getaddrinfo(host, port, &hints, &servinfo);
  if (rc != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    return std::make_pair(nullptr, -1);
  }

  for (struct addrinfo *p = servinfo; p != nullptr; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      continue;
    }

    if (to_bind) {
      rc = bind(sockfd, p->ai_addr, p->ai_addrlen);
      if (rc == -1) {
        fprintf(stderr, "bind :%d\n", rc);
        break;
      }
    }

    break;
  }

  if (sockfd == -1) {
    return std::make_pair(nullptr, -1);
  }

  return std::make_pair(servinfo, sockfd);
}

void udp_listener(int sockfd) {
  struct sockaddr_storage their_addr;
  socklen_t addr_len = sizeof(sockaddr_storage);
  char buf[MAXBUFLEN];
  int recvbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                           (struct sockaddr *)&their_addr, &addr_len);

  if (recvbytes == 0) {
    fprintf(stderr, "error on recvfrom");
    return;
  }

  char s[INET6_ADDRSTRLEN];
  printf("listener: got packet from: %s\n",
         inet_ntop(their_addr.ss_family,
                   get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s)));

  printf("listener: packet is %d bytes long\n", recvbytes);
  buf[recvbytes] = '\0';
  printf("listener: packet contains '%s'\n", buf);
  close(sockfd);
}

void udp_client(int sockfd, struct addrinfo *addr) {
  char buf[] = "hello from C\0";
  int sent_bytes =
      sendto(sockfd, buf, sizeof(buf), 0, addr->ai_addr, addr->ai_addrlen);
  fprintf(stderr, "client: sent bytes: %d\n", sent_bytes);
  close(sockfd);
}

int main() {
  std::thread server_thread([]() {
    std::pair<addrinfo *, int> server =
        create_socket_with_port(nullptr, PORT, true);
    if (server.second == -1)
      return;

    udp_listener(server.second);
  });

  std::thread client_thread([]() {
    std::pair<addrinfo *, int> client =
        create_socket_with_port("localhost", PORT, false);
    if (client.second == -1) {
      return;
    }

    udp_client(client.second, client.first);
  });

  server_thread.join();
  client_thread.join();
}
