/*
Questions to answer at top of client.c:

1. What is the address of the server it is trying to connect to (IP address and port number)?
   IP Address: 127.0.0.1 (localhost, defined by the ADDR macro)
   Port: 8000 (defined by the PORT macro)

2. Is it UDP or TCP? How do you know?
   It is TCP. You can tell because the socket is created with SOCK_STREAM
   (line: sfd = socket(AF_INET, SOCK_STREAM, 0)), which specifies a
   stream-oriented socket — i.e., TCP. UDP would use SOCK_DGRAM instead.
   Additionally, the code calls connect(), which is a TCP concept; UDP is
   connectionless and does not use connect() in the same way.

3. The client is going to send some data to the server. Where does it get
   this data from? How can you tell in the code?
   The client reads data from standard input (stdin / the keyboard). You can
   tell because the read() call uses STDIN_FILENO as its file descriptor:
       num_read = read(STDIN_FILENO, buf, BUF_SIZE)
   Whatever the user types at the terminal is read into buf and then written
   to the server socket with write(sfd, buf, num_read).

4. How does the client program end? How can you tell that in the code?
   The client ends when the user sends an empty line (just presses Enter),
   or signals EOF (Ctrl+D). The while loop condition is:
       while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1)
   num_read will be 1 when only a newline '\n' is read (empty line), causing
   the loop to exit. It also exits on EOF (num_read == 0) or an error
   (num_read == -1). After the loop, close(sfd) closes the socket and
   exit(EXIT_SUCCESS) terminates the program.
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64
#define ADDR "127.0.0.1"

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main() {
  struct sockaddr_in addr;
  int sfd;
  ssize_t num_read;
  char buf[BUF_SIZE];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  int res = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    handle_error("connect");
  }

  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1) {
    if (write(sfd, buf, num_read) != num_read) {
      handle_error("write");
    }
    printf("Just sent %zd bytes.\n", num_read);
  }

  if (num_read == -1) {
    handle_error("read");
  }

  close(sfd);
  exit(EXIT_SUCCESS);
}
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 64
#define PORT 8000
#define LISTEN_BACKLOG 32

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

// Shared counters for: total # messages, and counter of clients (used for
// assigning client IDs)
int total_message_count = 0;
int client_id_counter = 1;

// Mutexs to protect above global state.
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_info {
  int cfd;
  int client_id;
};

void *handle_client(void *arg) {
  // Extract client socket fd and client ID from the passed struct, then free it
  struct client_info *client = (struct client_info *)arg;
  int cfd = client->cfd;
  int client_id = client->client_id;
  free(client);

  char buf[BUF_SIZE];
  ssize_t num_read;

  // Loop: keep reading messages from this client until the socket closes
  while ((num_read = read(cfd, buf, BUF_SIZE - 1)) > 0) {
    // Null-terminate so we can safely print as a string
    buf[num_read] = '\0';

    // Strip trailing newline for cleaner output (optional but matches sample)
    if (num_read > 0 && buf[num_read - 1] == '\n') {
      buf[num_read - 1] = '\0';
    }

    // Increment total message count safely
    pthread_mutex_lock(&count_mutex);
    total_message_count++;
    int msg_num = total_message_count;
    pthread_mutex_unlock(&count_mutex);

    printf("Msg # %3d; Client ID %d: %s\n", msg_num, client_id, buf);
  }

  // read() returned 0 (client closed connection) or -1 (error) — either way,
  // we're done with this client
  printf("Ending thread for client %d\n", client_id);
  close(cfd);

  return NULL;
}

int main() {
  struct sockaddr_in addr;
  int sfd;

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    handle_error("listen");
  }

  for (;;) {
    // Block until a new client connects; cfd is the new per-client socket
    int cfd = accept(sfd, NULL, NULL);
    if (cfd == -1) {
      handle_error("accept");
    }

    // Assign a unique client ID (increment counter first, then capture value)
    pthread_mutex_lock(&client_id_mutex);
    int this_client_id = client_id_counter++;
    pthread_mutex_unlock(&client_id_mutex);

    printf("New client created! ID %d on socket FD %d\n", this_client_id, cfd);

    // Dynamically allocate a client_info struct to pass to the new thread
    struct client_info *info = malloc(sizeof(struct client_info));
    if (info == NULL) {
      handle_error("malloc");
    }
    info->cfd = cfd;
    info->client_id = this_client_id;

    // Spawn a new thread to handle this client
    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_client, info) != 0) {
      handle_error("pthread_create");
    }

    // Detach so thread resources are cleaned up automatically when it finishes
    pthread_detach(thread);
  }

  // Unreachable in practice (server runs forever), but good form
  if (close(sfd) == -1) {
    handle_error("close");
  }

  return 0;
}
