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


void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    chat_win = newwin(max_y, max_x, 0, 0);
    scrollok(chat_win, TRUE);
    wbkgd(chat_win, COLOR_PAIR(1));
    box(chat_win, 0, 0);
    mvwprintw(chat_win, 0, (max_x - 20) / 2, " Chat Server Running ");
    wrefresh(chat_win);
}


void update_chat_window(const char* message) {

    int max_y, max_x;
    getmaxyx(chat_win, max_y, max_x);
    int usable_width = max_x - 2;


    werase(chat_win);
    box(chat_win, 0, 0);


    char timestamp[32];
    time_t now = time(NULL);
    strftime(timestamp, sizeof(timestamp), "[%a %b %d %H:%M:%S %Y]", localtime(&now));


    char full_message[BUFFER_SIZE];
    snprintf(full_message, BUFFER_SIZE, "%s %s", timestamp, message);


    int msg_len = strlen(full_message);
    int lines_needed = (msg_len + usable_width - 1) / usable_width;
    if (lines_needed == 0) lines_needed = 1;


    static char display_lines[10][BUFFER_SIZE];
    if (message_count < 10) {
        if (message_count + lines_needed > 10) {
            int shift = (message_count + lines_needed) - 10;
            for (int i = shift; i < message_count; i++) {
                strcpy(display_lines[i - shift], display_lines[i]);
            }
            message_count -= shift;
        }
    }
    else {
        int shift = lines_needed;
        for (int i = shift; i < 10; i++) {
            strcpy(display_lines[i - shift], display_lines[i]);
        }
        message_count = 10 - lines_needed;
    }


    int pos = 0;
    for (int i = 0; i < lines_needed && message_count < 10; i++) {
        int chars_to_copy = usable_width;
        if (pos + chars_to_copy > msg_len) {
            chars_to_copy = msg_len - pos;
        }
        strncpy(display_lines[message_count], full_message + pos, chars_to_copy);
        display_lines[message_count][chars_to_copy] = '\0';
        pos += chars_to_copy;
        message_count++;
    }


    for (int i = 0; i < message_count; i++) {
        mvwprintw(chat_win, i + 1, 1, "%s", display_lines[i]);
    }

    wrefresh(chat_win);
}


void store_message(const char* message) {
    pthread_mutex_lock(&clients_mutex);
    if (message_count < MAX_LINES) {
        strcpy(last_messages[message_count], message);
        message_count++;
    }
    else {
        for (int i = 1; i < MAX_LINES; i++) {
            strcpy(last_messages[i - 1], last_messages[i]);
        }
        strcpy(last_messages[MAX_LINES - 1], message);
    }
    update_chat_window(message);
    pthread_mutex_unlock(&clients_mutex);
}


void broadcast_message(const char* message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sockfd != sender_sock) {
            send(clients[i].sockfd, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    update_chat_window(message);
}
