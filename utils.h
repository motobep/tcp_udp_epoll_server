#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UTILS
#define UTILS

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888

#define MAX_EPOLL_EVENTS 32

#define BUFFER_SIZE 1024

#define MAX_CLIENTS_COUNT 1024

void err(const char *msg) {
  perror(msg);
  exit(1);
}

void fill_server_sockaddr_in(struct sockaddr_in *server_addr) {
  struct in_addr local_ip;
  int ok = inet_pton(AF_INET, SERVER_IP, &local_ip);
  if (ok <= 0)
    err("Invalid address");

  server_addr->sin_addr = local_ip;
  server_addr->sin_port = htons(SERVER_PORT);
  server_addr->sin_family = AF_INET;

  printf("Filled server sockaddr_in for '%s:%d'\n", SERVER_IP, SERVER_PORT);
}

ssize_t my_send(int sockfd, char *message) {
  printf("send: %s\n", message);
  return send(sockfd, message, strlen(message), 0);
}

ssize_t my_recv(int sockfd, char *buffer) {
  int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
  if (bytes_received < 0)
    err("Receive failed");

  buffer[bytes_received] = 0;
  printf("recv: %s\n", buffer);
  return bytes_received;
}

ssize_t my_sendto(int sockfd, char *message, struct sockaddr_in *server_addr) {
  printf("sendto: %s\n", message);
  return sendto(sockfd, message, strlen(message), 0,
                (struct sockaddr *)server_addr, sizeof(*server_addr));
}

int my_recvfrom(int sockfd, char *buffer, struct sockaddr_in *server_addr) {
  socklen_t addr_len = sizeof(*server_addr);
  ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr *)server_addr, &addr_len);
  if (bytes_received < 0)
    err("Receive failed");

  buffer[bytes_received] = 0;
  printf("recvfrom: %s\n", buffer);
  return bytes_received;
}

int ALL_CLIENTS_COUNT = 0;
char *all_clients[MAX_CLIENTS_COUNT];

int contains_string(char *arr[], int size, const char *str) {
  for (int i = 0; i < size; i++) {
    if (strcmp(arr[i], str) == 0) {
      return 1;
    }
  }
  return 0;
}

void add_to_all_clients(struct sockaddr_in *client_addr) {
  const int ip_port_len = INET_ADDRSTRLEN + 1 + 5;
  char *client_str = (char *)malloc(ip_port_len);

  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(client_addr->sin_addr), ip_str, INET_ADDRSTRLEN);
  int port = ntohs(client_addr->sin_port);
  sprintf(client_str, "%s:%d", ip_str, port);
  printf("add_to_all_clients() - IP:PORT=%s\n", client_str);

  if (!contains_string(all_clients, ALL_CLIENTS_COUNT, client_str)) {
    if (ALL_CLIENTS_COUNT >= MAX_CLIENTS_COUNT) {
      err("Exceeded MAX_CLIENTS_COUNT const");
    }
    all_clients[ALL_CLIENTS_COUNT] = client_str;
    ALL_CLIENTS_COUNT++;
  }
}

#endif
