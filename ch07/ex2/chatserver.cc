#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

#define PORT "9000"
#define BACKLOG 16

inline void ERR(const char *msg) { fprintf(stderr, "[server] %s\n", msg); }

int create_listener() {
  struct addrinfo hint = {
      .ai_flags = AI_PASSIVE,
      .ai_family = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM,
  };

  int rc = 0;
  struct addrinfo *servinfo;

  // get addrinfo for localhost
  if ((rc = getaddrinfo(NULL, PORT, &hint, &servinfo) != 0)) {
    fprintf(stderr, "[server] Failed to get serverinf: %s\n", gai_strerror(rc));
    return -1;
  }

  int sockfd = -1;
  for (struct addrinfo *p = servinfo; p != nullptr; p = p->ai_next) {
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
                    servinfo->ai_protocol);

    if (sockfd == -1) {
      continue;
    }

    int yes = 1;
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc != 0) {
      fprintf(stderr, "[server] Failed to apply SO_REUSEADDR to the socket\n");
      return -1;
    }

    rc = bind(sockfd, p->ai_addr, p->ai_addrlen);
    if (rc == -1) {
      close(sockfd);
      fprintf(stderr, "[server] Failed to bind to the socket\n");
      return -1;
    }

    break;
  }

  freeaddrinfo(servinfo);
  if (sockfd == -1) {
    fprintf(stderr, "[server] Failed to create a socket file descriptor\n");
    return -1;
  }

  rc = listen(sockfd, BACKLOG);
  if (rc == -1) {
    close(sockfd);
    fprintf(stderr, "[server] Failed to set connection backlog: %d\n", errno);
    return -1;
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

int main() {
  int MAX_FD = 16;
  struct pollfd *conns =
      (struct pollfd *)malloc(sizeof(struct pollfd *) * MAX_FD);

  int fds = -1;
  int listener = create_listener();
  if (listener == -1) {
    exit(1);
  }

  fprintf(stderr, "[server] Listening on port %s, fd:%d\n", PORT, listener);

  conns[++fds] = {
      .fd = listener,
      .events = POLLIN,
  };

  while (1) {
    // wait for something to be ready
    int poll_count = poll(conns, fds + 1, -1);

    if (poll_count == -1) {
      fprintf(stderr, "[server] Spurious wake\n");
      exit(1);
    }

    // found something
    for (int i = 0; i <= fds; i++) {
      if (conns[i].revents & POLLIN) {
        // Something is ready to read

        fprintf(stderr, "[server] Got a ready from fd:%d\n", conns[i].fd);
        if (conns[i].fd == listener) {
          // The listener is ready to accept a new connection
          struct sockaddr_storage remote_addr;
          socklen_t addrlen = sizeof(remote_addr);

          int rfd = accept(listener, (struct sockaddr *)&remote_addr, &addrlen);
          if (rfd == -1) {
            fprintf(stderr,
                    "[server] Failed to accept the client connection\n");
            continue;
          }

          // Add the rfd to the pollfd list
          if (fds + 1 == MAX_FD) {
            // at capacity
            MAX_FD <<= 1;
            conns = (struct pollfd *)realloc(conns,
                                             sizeof(struct pollfd *) * MAX_FD);
          }

          conns[++fds] = {
              .fd = rfd,
              .events = POLLIN,
          };

          std::pair<char *, unsigned short> raddr =
              get_client_address(&remote_addr);
          fprintf(stderr, "[server] Got a connection from %s:%d\n", raddr.first,
                  raddr.second);
        } else {
          // One of the connections are ready to read from
          char buf[1024];
          int read_bytes = recv(conns[i].fd, buf, sizeof(buf), 0);
          if (read_bytes <= 0) {
            // There is some error or the connection has closed,
            // need to close and remove it
            fprintf(stderr, "Nothing to read, closing %d\n", conns[i].fd);
            close(conns[i].fd);
            conns[i] = conns[fds--];
            continue;
          }

          // We got data, send it to everyone
          for (int c = 0; c <= fds; c++) {
            if (conns[c].fd == listener || conns[c].fd == conns[i].fd) {
              continue;
            }

            int preamble = 0;
            char preamble_buf[8];
            preamble = sprintf(preamble_buf, "[%d] ", conns[i].fd);
            send(conns[c].fd, preamble_buf, preamble, 0);
            send(conns[c].fd, buf, read_bytes, 0);
          }
          printf("[server] (%d): %s", conns[i].fd, buf);
          std::fill_n(buf, read_bytes, 0);
        }
      }
    }
  }

  return 0;
}
