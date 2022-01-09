#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  struct pollfd pfds[1];
  pfds[0] = {
      .fd = 0,
      .events = POLLIN,
  };

  fprintf(stderr, "Hit return or wait 5 seconds for timeout\n");

  int num_events = poll(pfds, 1, 5000);

  if (num_events == 0) {
    fprintf(stderr, "Poll timed out\n");
    return 0;
  }

  int ready_to_read = pfds[0].revents & POLLIN;

  if (ready_to_read) {
    fprintf(stderr, "File descriptor ready to read\n");
    char buf[1024];
    int bytes_read = read(pfds[0].fd, buf, sizeof(buf));
    if (bytes_read != -1) {
      buf[bytes_read] = '\0';
      printf("%s", buf);
      // buf[bytes_read] = '\0';
      printf("Input (%d bytes): %s\n", bytes_read, buf);
    } else {
      fprintf(stderr, "Failed to read the value\n", errno);
    }
  } else {
    fprintf(stderr, "Spurious wake from poll(2)\n");
  }

  return 0;
}
