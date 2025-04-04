/*
FILE          : server.c
PROJECT       : Chat Server
PROGRAMMER    : Anchita Kakroa, Uttam Arora, Aryan Malviya
FIRST VERSION : 31 March 2025
DESCRIPTION   : This file implements thread chat server. It handles client connections. manages clients and
                perform handling and also termination. .
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ncurses.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "chat_protocol.h"
#include "server_utils.h"

#define SERVER_PORT 8080

int server_socket = -1;

int main() {
    struct sockaddr_in server_addr;

    //all signals are blocked so there are no interruptions
    sigset_t mask, old_mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, &old_mask);


    struct sigaction sa; // signal handlers are set to handle termination
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //socket created for server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        return 1;
    }

    //socktion option are set
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(server_socket);
        return 1;
    }

    //server addr struct set up
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    //socket is bind to server address
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        return 1;
    }

    //start listening for connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        return 1;
    }

    //tells the user that server is running
    printf("Server running on 0.0.0.0:%d\n", SERVER_PORT);
    printf("Press Ctrl+C to stop the server\n");


    init_ncurses(); //ncurses lib


    sigprocmask(SIG_SETMASK, &old_mask, NULL);


    while (server_running) { //loop tp handle client connections
        fd_set readfds;
        struct timeval tv;

        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);


        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(server_socket + 1, &readfds, NULL, NULL, &tv); //use select () to monitor socket

        if (!server_running) break;

        if (activity < 0 && errno != EINTR) {
            perror("Select error");
            continue;
        }

        if (activity > 0 && FD_ISSET(server_socket, &readfds)) { // in this server is ready for new connections
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len); //accepts new connection
            if (client_socket < 0) {
                if (errno != EINTR && errno != EWOULDBLOCK && server_running) {
                    perror("Error accepting connection");
                }
                continue;
            }

            pthread_mutex_lock(&clients_mutex);  //lock the mutex before updating shared client indo
            if (client_count >= MAX_CLIENTS) {
                const char* msg = "Server is full. Please try again later.\n";
                send(client_socket, msg, strlen(msg), 0);
                close(client_socket);
                pthread_mutex_unlock(&clients_mutex);
                continue;
            }

            clients[client_count].sockfd = client_socket; //adds the new client to the list
            clients[client_count].address = client_addr;

            if (pthread_create(&clients[client_count].thread, NULL, handle_client, &clients[client_count]) != 0) {
                perror("Error creating thread");
                close(client_socket);
                pthread_mutex_unlock(&clients_mutex);
                continue;
            }

            client_count++; //performs cleanup
            pthread_mutex_unlock(&clients_mutex);
        }
    }


    cleanup_server();
    close(server_socket);

    return 0;
}
