#include <arpa/inet.h>

#include "utils.h"

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    err("Socket creation failed");

  fill_server_sockaddr_in(&server_addr);

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    err("Connection failed");

  my_send(sockfd, "/stats");
  my_recv(sockfd, buffer);

  my_send(sockfd, "/time");
  my_recv(sockfd, buffer);

  my_send(sockfd, "time");
  my_recv(sockfd, buffer);

  my_send(sockfd, "/stats");
  my_recv(sockfd, buffer);

  my_send(sockfd, "/shutdown");
  my_recv(sockfd, buffer);

  my_send(sockfd, "after shutdown");
  my_recv(sockfd, buffer);

  return 0;
}
