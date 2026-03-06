
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define NUM_MSG_PER_CLIENT 5

char *messages[NUM_MSG_PER_CLIENT] = {"Hello", "Apple", "Car", "Green", "Dog"};

int main() {
  int sock;
  struct sockaddr_in serv_addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

  connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  for (int i = 0; i < NUM_MSG_PER_CLIENT; i++) {
    send(sock, messages[i], strlen(messages[i]) + 1, 0);
    printf("Sent: %s\n", messages[i]);
    usleep(100000);
  }

  close(sock);

  return 0;
}

/*
Questions to answer at top of server.c:

Understanding the Client:

1. How is the client sending data to the server? What protocol?
The client sends data using TCP protocol. This is done using SOCK_STREAM
sockets, which provide reliable, ordered, connection-based communication.

2. What data is the client sending to the server?
The client sends strings from the messages array: "Hello", "Apple", "Car",
"Green", and "Dog".

Understanding the Server:

1. Explain the argument that the run_acceptor thread is passed as an argument.
The run_acceptor thread is passed the server socket file descriptor. This socket
is used to accept incoming client connections.

2. How are received messages stored?
Received messages are stored in a linked list. Each message is placed into a
node, and nodes are added safely using a mutex.

3. What does main() do with the received messages?
main() waits until all messages from all clients are received, then prints each
message and shuts down the server cleanly.

4. How are threads used in this sample code?
One thread runs the acceptor, which listens for new clients. A new thread is
created for each client connection to receive messages concurrently.

Explain the use of non-blocking sockets in this lab.

How are sockets made non-blocking?
Sockets are made non-blocking using fcntl() and setting the O_NONBLOCK flag.

What sockets are made non-blocking?
The server socket and client sockets are made non-blocking.

Why are these sockets made non-blocking? What purpose does it serve?
Non-blocking sockets allow threads to continue running without waiting for
socket operations to complete. This prevents the server from freezing and allows
multiple clients to be handled concurrently.
*/

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 4
#define NUM_MSG_PER_CLIENT 5
#define BUFFER_SIZE 1024

typedef struct node {
  char message[BUFFER_SIZE];
  struct node *next;
} node_t;

typedef struct {
  int socket;
  pthread_t thread;
  int run;
} client_t;

node_t *head = NULL;

client_t clients[MAX_CLIENTS];

int num_clients = 0;
int num_messages = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void add_to_list(char *message) {
  node_t *new_node = malloc(sizeof(node_t));

  strcpy(new_node->message, message);

  new_node->next = head;

  head = new_node;

  num_messages++;
}

void *run_client(void *arg) {
  client_t *client = (client_t *)arg;

  char buffer[BUFFER_SIZE];

  while (client->run) {
    int bytes = recv(client->socket, buffer, BUFFER_SIZE, 0);

    if (bytes > 0) {
      pthread_mutex_lock(&mutex);

      add_to_list(buffer);

      pthread_mutex_unlock(&mutex);
    } else {
      break;
    }
  }

  return NULL;
}

void *run_acceptor(void *arg) {
  int server_socket = *(int *)arg;

  struct sockaddr_in client_addr;

  socklen_t addr_len = sizeof(client_addr);

  while (num_clients < MAX_CLIENTS) {
    int client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);

    if (client_socket >= 0) {
      printf("Client connected!\n");

      clients[num_clients].socket = client_socket;

      clients[num_clients].run = 1;

      pthread_create(&clients[num_clients].thread, NULL, run_client,
                     &clients[num_clients]);

      num_clients++;
    }
  }

  printf("Not accepting any more clients!\n");

  for (int i = 0; i < num_clients; i++) {
    clients[i].run = 0;

    pthread_join(clients[i].thread, NULL);

    close(clients[i].socket);
  }

  return NULL;
}

int main() {
  int server_socket;

  struct sockaddr_in address;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  bind(server_socket, (struct sockaddr *)&address, sizeof(address));

  listen(server_socket, MAX_CLIENTS);

  int flags = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

  pthread_t acceptor_thread;

  pthread_create(&acceptor_thread, NULL, run_acceptor, &server_socket);

  int total_messages = MAX_CLIENTS * NUM_MSG_PER_CLIENT;

  while (1) {
    pthread_mutex_lock(&mutex);

    if (num_messages >= total_messages) {
      pthread_mutex_unlock(&mutex);
      break;
    }

    pthread_mutex_unlock(&mutex);
  }

  node_t *current = head;

  while (current != NULL) {
    printf("Collected: %s\n", current->message);

    current = current->next;
  }

  printf("All messages were collected!\n");

  pthread_join(acceptor_thread, NULL);

  close(server_socket);

  return 0;
}
