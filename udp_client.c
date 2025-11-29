#include <arpa/inet.h>

#include "utils.h"

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    err("Socket creation failed");

  fill_server_sockaddr_in(&server_addr);

  my_sendto(sockfd, "/stats", &server_addr);
  my_recvfrom(sockfd, buffer, &server_addr);

  my_sendto(sockfd, "/time", &server_addr);
  my_recvfrom(sockfd, buffer, &server_addr);

  my_sendto(sockfd, "time", &server_addr);
  my_recvfrom(sockfd, buffer, &server_addr);

  my_sendto(sockfd, "/stats", &server_addr);
  my_recvfrom(sockfd, buffer, &server_addr);

  my_sendto(sockfd, "/shutdown", &server_addr);
  my_recvfrom(sockfd, buffer, &server_addr);

  my_sendto(sockfd, "after shutdown", &server_addr);
  my_recvfrom(sockfd, buffer, &server_addr);

  return 0;
}
