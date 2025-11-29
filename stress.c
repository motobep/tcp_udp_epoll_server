#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

#define NUMBER_OF_CLIENTS 64

void create_client(int client_id) {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[256];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    err("Socket creation failed");

  fill_server_sockaddr_in(&server_addr);

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    err("Connection failed");

  int to_sleep = 0;
  if (client_id % 3 == 0) {
    to_sleep = (10 + 3 * 80 - (80 * client_id % 3));
  }
  if (client_id % 3 == 1) {
    to_sleep = (10 + 3 * 80 + (80 * client_id % 3));
  }
  if (client_id % 3 == 1) {
    to_sleep = (10 + 3 * 80 - 2 * (80 * client_id % 3));
  }
  usleep(100 * (to_sleep));

  char message[256];
  snprintf(message, sizeof(message), "Client Hello: %d", client_id);

  my_send(sockfd, message);

  my_recv(sockfd, buffer);

  my_send(sockfd, "/stats");
  my_recv(sockfd, buffer);

  // close(sockfd); // OS will handle this
}

int main() {
  for (int i = 0; i < NUMBER_OF_CLIENTS; i++) {
    pid_t pid = fork();

    if (pid == 0) {
      // Child
      create_client(i);
      exit(0);
    } else if (pid < 0) {
      perror("Fork failed");
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < NUMBER_OF_CLIENTS; i++) {
    wait(NULL);
  }

  return 0;
}
