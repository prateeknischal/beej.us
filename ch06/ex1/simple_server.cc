#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#define PORT "9000"
#define BACKLOG 10

// Creates a socket file descriptor and sets it up to listen on the specified
// PORT.
int create_listener() {
  struct addrinfo hint = {
      .ai_flags = AI_PASSIVE,
      .ai_family = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM,
  };

  int rc = 0;
  struct addrinfo *servinfo;

  if ((rc = getaddrinfo(NULL, PORT, &hint, &servinfo)) != 0) {
    fprintf(stderr, "failed to getaddrinfo: %s\n", gai_strerror(rc));
    return 1;
  }

  int sockfd = -1;
  for (struct addrinfo *p = servinfo; p != nullptr; p = p->ai_next) {
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
                    servinfo->ai_protocol);
    if (sockfd == -1) {
      fprintf(stderr, "failed to create a socket: %d\n", errno);
      continue;
    }

    int yes = 1;
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc != 0) {
      fprintf(stderr, "failed to set socket option SO_REUSEADDR\n");
      exit(1);
    }

    rc = bind(sockfd, p->ai_addr, p->ai_addrlen);
    if (rc == -1) {
      close(sockfd);
      fprintf(stderr, "failed to bind: %d\n", errno);
      exit(1);
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (sockfd == -1) {
    fprintf(stderr, "unable to bind to any address\n");
    exit(1);
  }

  rc = listen(sockfd, BACKLOG);
  if (rc == -1) {
    close(sockfd);
    fprintf(stderr, "failed to set connection backlog: %d\n", errno);
    exit(1);
  }

  return sockfd;
}

// Accepts a pointer to sockaddr struct and returns in_addr or in6_addr instance
// depending on what the client address is.
std::pair<char *, unsigned short>
get_client_address(struct sockaddr_storage *a) {
  char buf[INET6_ADDRSTRLEN];
  struct sockaddr *p = (struct sockaddr *)a;

  if (p->sa_family == AF_INET) {
    struct sockaddr_in *sin = (struct sockaddr_in *)p;
    inet_ntop(a->ss_family, &sin->sin_addr, buf, sizeof(buf));
    return std::make_pair(buf, ntohs(sin->sin_port));
  }

  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)p;
  inet_ntop(a->ss_family, &sin6->sin6_addr, buf, sizeof(buf));
  return std::make_pair(buf, ntohs(sin6->sin6_port));
}

// Accept and handle the incoming requests.
void handle_request(int sockfd) {
  char in[INET6_ADDRSTRLEN];
  while (1) {
    struct sockaddr_storage client_address;
    socklen_t sin_size = sizeof(client_address);

    fprintf(stderr, "[server] listening for connection\n");
    int fd = accept(sockfd, (struct sockaddr *)&client_address, &sin_size);
    if (fd == -1) {
      fprintf(stderr, "[server] failed to accept client connection\n");
      continue;
    }

    std::pair<char *, unsigned short> pr = get_client_address(&client_address);
    fprintf(stdout, "[server] got a connection from: %s:%d\n", pr.first,
            pr.second);

    if (!fork()) {
      // close the copy of socket file descriptor in the child process when the
      // process if forked.
      close(sockfd);

      char buf[1 << 12];
      int recv_size = 0;
      memset(buf, 0, sizeof(buf));

      while (1) {
        recv_size = recv(fd, buf, sizeof(buf), 0);
        if (recv_size == 0) {
          break;
        }

        // echo the incoming request
        if (send(fd, buf, sizeof(buf), 0) == -1) {
          fprintf(stderr, "[%s] failed to send msg: %d\n", in, errno);
          exit(0);
        }
      }
    }

    // close the client connection
    close(fd);
  }
}

int main() {
  int sockfd = create_listener();
  handle_request(sockfd);
  return 0;
}
