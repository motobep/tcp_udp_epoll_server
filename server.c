#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

int CLIENTS_COUNT = 0;

struct sockaddr_in server_addr;
socklen_t server_addr_len = sizeof(struct sockaddr_in);

struct sockaddr_in client_addr;
socklen_t client_addr_len = sizeof(struct sockaddr_in);

void handle_reqest(int sockfd, char *req, struct sockaddr_in *client_addr) {
  /*
    /time — return date in format "1111-11-11 11:11:11"
    /stats — statistics
        (clients connected now and all clients that have ever been connected)
    /shutdown — shutdown server
  */

  if (strcmp(req, "/time") == 0) {
    time_t t;
    time(&t);

    struct tm *tm_info;
    tm_info = localtime(&t);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    if (client_addr == 0) {
      my_send(sockfd, buffer);
    } else {
      my_sendto(sockfd, buffer, client_addr);
    }
    return;
  }

  if (strcmp(req, "/stats") == 0) {
    char *template = "Connected clients: %d. Total number of clients: %d";
    char buffer[128]; // Enough size
    sprintf(buffer, template, CLIENTS_COUNT, ALL_CLIENTS_COUNT);

    if (client_addr == 0) {
      my_send(sockfd, buffer);
    } else {
      my_sendto(sockfd, buffer, client_addr);
    }
    return;
  }

  if (strcmp(req, "/shutdown") == 0) {
    printf("shutdown: just exiting"); // OS will free resources
    exit(0);
    return;
  }

  if (client_addr == 0) {
    my_send(sockfd, req);
  } else {
    my_sendto(sockfd, req, client_addr);
  }
}

int make_tcp_listener() {
  fill_server_sockaddr_in(&server_addr);

  int listener_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  // printf("tcp_listener_fd: %d\n", tcp_listener_fd);
  if (listener_fd == -1)
    err("Bad socket");

  int opt_val = 1;

  int ok1 = setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val,
                       sizeof(opt_val));
  if (ok1 != 0)
    err("Bad setsockopt");

  int ok2 =
      bind(listener_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (ok2 != 0)
    err("Bad bind");

  printf("Tcp Start listening\n");
  listen(listener_fd, SOMAXCONN);
  return listener_fd;
}

int make_udp_listener() {
  fill_server_sockaddr_in(&server_addr);

  int listener_fd = socket(AF_INET, SOCK_DGRAM, 0);
  // printf("udp_listener_fd: %d\n", listener_fd);
  if (listener_fd == -1)
    err("Bad socket");

  int opt_val = 1;

  int ok1 = setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val,
                       sizeof(opt_val));
  if (ok1 != 0)
    err("Bad setsockopt");

  int ok2 =
      bind(listener_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (ok2 != 0)
    err("Bad bind");

  printf("Udp Start listening\n");
  return listener_fd;
}

int main() {
  char msg[BUFFER_SIZE];
  int msgLen;
  int new_fd;

  int eventsCount = 0;
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_fd error");
    exit(1);
  }
  struct epoll_event epollEvents[MAX_EPOLL_EVENTS];

  struct epoll_event server_evt;
  server_evt.events = EPOLLIN;

  // Set TCP listener
  int tcp_listener = make_tcp_listener();
  server_evt.data.fd = tcp_listener;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_listener, &server_evt) == -1)
    err("Bad tcp_listener epoll_ctl");

  // Set UDP listener
  int udp_listener = make_udp_listener();
  server_evt.data.fd = udp_listener;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_listener, &server_evt) == -1)
    err("Bad udp_listener epoll_ctl");

  struct epoll_event new_epoll_evt;

  int sockfd;

  printf("Epolling\n");

  while (1) {
    const int epoll_timeout = -1;
    // const int epoll_timeout = 500;
    eventsCount =
        epoll_wait(epoll_fd, epollEvents, MAX_EPOLL_EVENTS, epoll_timeout);
    if (epoll_timeout != -1 && eventsCount == 0) {
      printf("Non-blocking. Doing other stuff\n");
    }

    for (int i = 0; i < eventsCount; i++) {
      // printf("Event %d for fd %d\n", epollEvents[i].events,
      //        epollEvents[i].data.fd);
      uint32_t events = epollEvents[i].events;
      if (events & EPOLLIN) {
        sockfd = epollEvents[i].data.fd;
        if (sockfd == tcp_listener) {
          printf("Accept connection\n");
          new_fd = accept(tcp_listener, (struct sockaddr *)&client_addr,
                          &client_addr_len);

          if (new_fd == -1)
            err("Bad new_fd");

          if (new_fd > 0) {
            new_epoll_evt.data.fd = new_fd;
            new_epoll_evt.events = EPOLLIN;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &new_epoll_evt) ==
                -1)
              err("Bad new_fd epoll_ctl");

            // printf("Added new_fd+events: %d\n", new_fd);
            add_to_all_clients(&client_addr);
            CLIENTS_COUNT++;
          } else {
            perror("accept error");
          }
        } else if (sockfd == udp_listener) {
          // printf("Handle udp client message\n");

          msgLen = recvfrom(sockfd, msg, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)&client_addr, &client_addr_len);

          if (msgLen > 0) {
            msg[msgLen] = 0;
            printf("udp msg[%d]: %s\n\n", msgLen, msg);
            add_to_all_clients(&client_addr);

            // Treat datagram as a temporary client
            CLIENTS_COUNT++;
            handle_reqest(sockfd, msg, &client_addr);
            CLIENTS_COUNT--;
          }
        } else {
          // printf("Handle tcp client message\n");

          msgLen = recv(sockfd, msg, sizeof(msg), 0);
          if (msgLen > 0) {
            msg[msgLen] = 0;
            printf("tcp msg[%d]: %s\n\n", msgLen, msg);
            handle_reqest(sockfd, msg, NULL);
          } else {
            CLIENTS_COUNT--;
            close(sockfd);
          }
        }
      } else if (events & EPOLLERR || events & EPOLLHUP) {
        CLIENTS_COUNT--;
        close(sockfd);
      }
    }
  }

  return 0;
}
