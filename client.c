#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ncurses.h>
#include <signal.h>
#include "../include/chat_protocol.h"
#include "../include/client_utils.h"

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int sockfd;
char username[50];
WINDOW* chat_win, * input_win;
char chat_history[MAX_LINES][BUFFER_SIZE];
int line_count = 0;
volatile sig_atomic_t client_running = 1;

void cleanup_client(void) {
    if (sockfd != -1) {
        // sending message before closing chat
        const char* exit_msg = ">>bye<<";
        send(sockfd, exit_msg, strlen(exit_msg), 0);
        usleep(100000);


        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        sockfd = -1;
    }


    if (chat_win) {
        delwin(chat_win);
        chat_win = NULL;
    }
    if (input_win) {
        delwin(input_win);
        input_win = NULL;
    }
    endwin();


    system("stty sane");
    printf("\033[?25h");
    printf("\033[0m");
    printf("\033c");
    fflush(stdout);
}

void handle_signal(int sig) {
    client_running = 0;
    cleanup_client();
    exit(0);
}