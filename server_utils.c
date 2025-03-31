#include "server_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

char last_messages[MAX_LINES][BUFFER_SIZE];
int message_count = 0;


WINDOW* chat_win, * input_win;

volatile sig_atomic_t server_running = 1;
extern int server_socket;

void handle_signal(int sig) {

    server_running = 0;


    if (server_socket != -1) {
        shutdown(server_socket, SHUT_RDWR);
        close(server_socket);
        server_socket = -1;
    }


    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    cleanup_server();
    exit(0);
}

void handle_signal(int sig) {

    server_running = 0;


    if (server_socket != -1) {
        shutdown(server_socket, SHUT_RDWR);
        close(server_socket);
        server_socket = -1;
    }


    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    cleanup_server();
    exit(0);
}

void cleanup_server(void) {
    const char* shutdown_msg = "Server is shutting down. Goodbye!\n";


    pthread_mutex_lock(&clients_mutex);


    for (int i = 0; i < client_count; i++) {
        if (clients[i].sockfd != -1) {
            send(clients[i].sockfd, shutdown_msg, strlen(shutdown_msg), MSG_NOSIGNAL);
            shutdown(clients[i].sockfd, SHUT_RDWR);
            close(clients[i].sockfd);
            clients[i].sockfd = -1;
            pthread_cancel(clients[i].thread);
        }
    }


    client_count = 0;
    pthread_mutex_unlock(&clients_mutex);


    if (chat_win) {
        werase(chat_win);
        wrefresh(chat_win);
        delwin(chat_win);
        chat_win = NULL;
    }
    if (input_win) {
        werase(input_win);
        wrefresh(input_win);
        delwin(input_win);
        input_win = NULL;
    }


    clear();
    refresh();
    endwin();


    system("stty sane");


    printf("\033[?25h");
    printf("\033[0m");
    printf("\033c");
    printf("\nServer shut down successfully\n");
    fflush(stdout);
}
