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

void* receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    while (client_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            if (client_running) {
                update_chat_window("*** Server disconnected ***");
                client_running = 0;
            }
            break;
        }
        update_chat_window(buffer);
    }
    return NULL;
}

void* send_messages(void* arg) {
    char message[BUFFER_SIZE];
    int bytes_sent;

    while (client_running) {

        werase(input_win);
        box(input_win, 0, 0);
        mvwprintw(input_win, 1, 1, "Message: ");
        wrefresh(input_win);


        echo();

        wmove(input_win, 1, 10);
        wrefresh(input_win);


        wgetnstr(input_win, message, BUFFER_SIZE - 10);


        noecho();


        if (strcmp(message, ">>bye<<") == 0) {
            client_running = 0;
            cleanup_client();
            break;
        }


        size_t prefix_len = strlen("You: ");
        size_t max_msg_len = BUFFER_SIZE - prefix_len - 1;

        if (strlen(message) > max_msg_len) {
            message[max_msg_len] = '\0';
        }


        char local_message[BUFFER_SIZE];
        snprintf(local_message, BUFFER_SIZE, "You: %.*s",
            (int)max_msg_len, message);
        update_chat_window(local_message);


        bytes_sent = send(sockfd, message, strlen(message), 0);
        if (bytes_sent <= 0) {
            if (client_running) {
                mvwprintw(chat_win, 1, 1, "Error sending message");
                wrefresh(chat_win);
            }
            break;
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    struct sockaddr_in server_addr;
    pthread_t recv_thread, send_thread;


    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);


    initscr();
    cbreak();
    noecho();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);


    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    chat_win = newwin(max_y - 3, max_x, 0, 0);
    input_win = newwin(3, max_x, max_y - 3, 0);
    scrollok(chat_win, TRUE);


    wbkgd(chat_win, COLOR_PAIR(1));
    wbkgd(input_win, COLOR_PAIR(2));


    box(chat_win, 0, 0);
    box(input_win, 0, 0);
    mvwprintw(chat_win, 0, (max_x - 12) / 2, " Chat Room ");

    // getting username
    mvwprintw(input_win, 1, 1, "Enter username: ");
    wrefresh(input_win);
    echo();
    mvwgetnstr(input_win, 1, 16, username, 49);
    noecho();

    // getting server ip
    char server_ip[16];
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "Enter server IP (default 127.0.0.1): ");
    wrefresh(input_win);
    echo();
    mvwgetnstr(input_win, 1, 35, server_ip, 15);
    noecho();

    if (strlen(server_ip) == 0) {
        strcpy(server_ip, "127.0.0.1");
    }

    // conecting to the server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        endwin();
        printf("Invalid IP address\n");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        endwin();
        perror("Connection failed");
        return 1;
    }


    send(sockfd, username, strlen(username), 0);


    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "Message: ");
    wrefresh(input_win);


    pthread_create(&recv_thread, NULL, receive_messages, NULL);
    pthread_create(&send_thread, NULL, send_messages, NULL);


    pthread_join(send_thread, NULL);


    cleanup_client();
    return 0;
}
