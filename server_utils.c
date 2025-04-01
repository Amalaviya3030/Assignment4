#include "server_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>


Client clients[MAX_CLIENTS]; //array to store the client that are connected
int client_count = 0; //tracks it
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

char last_messages[MAX_LINES][BUFFER_SIZE]; //stores the last messages
int message_count = 0; //number of stored messages


WINDOW* chat_win, * input_win;

volatile sig_atomic_t server_running = 1;
extern int server_socket;

/*
FUNCTION    : handle_signal
DESCRIPTION : handles the signals so that shut down server and performs cleanups
PARAMETERS  : int sig
RETURns : none
*/

void handle_signal(int sig) {

    server_running = 0;

    //closes the server if its open
    if (server_socket != -1) {
        shutdown(server_socket, SHUT_RDWR);
        close(server_socket);
        server_socket = -1;
    }

    //reset the handling to its original bahavior
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    cleanup_server(); //clean up
    exit(0);  //exit
}

/*
FUNCTION    : cleanup_server
DESCRIPTION : clean up is done when the server shut downs
PARAMETERS  : None
RETURNS     : None
*/

void cleanup_server(void) {
    const char* shutdown_msg = "Server is shutting down. Goodbye!\n";


    pthread_mutex_lock(&clients_mutex);


    for (int i = 0; i < client_count; i++) { //loop through the client connections and shuts them all down
        if (clients[i].sockfd != -1) {
            send(clients[i].sockfd, shutdown_msg, strlen(shutdown_msg), MSG_NOSIGNAL);
            shutdown(clients[i].sockfd, SHUT_RDWR);
            close(clients[i].sockfd);
            clients[i].sockfd = -1;
            pthread_cancel(clients[i].thread);
        }
    }


    client_count = 0; //reset client count
    pthread_mutex_unlock(&clients_mutex);


    if (chat_win) { //clean up ncurses
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

    //prints and rstores cursor and terminal state
    printf("\033[?25h");
    printf("\033[0m");
    printf("\033c");
    printf("\nServer shut down successfully\n");
    fflush(stdout);
}

/*
FUNCTION    : init_ncurses
DESCRIPTION : starts the ncurse to set up terminal jandling and diaplay the status of server to the user
PARAMETERS  : None
RETURNS: none
*/

void init_ncurses() {
    initscr(); //initalizes the lib
    cbreak(); //line buffering disabled
    noecho();
    start_color();// for color
    init_pair(1, COLOR_WHITE, COLOR_BLUE);

    //chat ui

    int max_y, max_x; //new window for chat messages
    getmaxyx(stdscr, max_y, max_x); //
    chat_win = newwin(max_y, max_x, 0, 0);
    scrollok(chat_win, TRUE);
    wbkgd(chat_win, COLOR_PAIR(1));
    box(chat_win, 0, 0);
    mvwprintw(chat_win, 0, (max_x - 20) / 2, " Chat Server Running ");
    wrefresh(chat_win);
}

/*
FUNCTION    : update_chat_window
DESCRIPTION : updayes the chat window with the lates messages
PARAMETERS  : const char *message - The message to be displayed in the chat window.
RETURNS     : None
*/

void update_chat_window(const char* message) {

    int max_y, max_x;
    getmaxyx(chat_win, max_y, max_x); //window size
    int usable_width = max_x - 2;  //border space


    werase(chat_win);
    box(chat_win, 0, 0);


    char timestamp[32]; //timestamp will be added with the message
    time_t now = time(NULL);
    strftime(timestamp, sizeof(timestamp), "[%a %b %d %H:%M:%S %Y]", localtime(&now));


    char full_message[BUFFER_SIZE];
    snprintf(full_message, BUFFER_SIZE, "%s %s", timestamp, message);

    //number of lines need to diaply the message
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

    //split the message into how many lines can be displayed
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

    //displayes all the lines
    for (int i = 0; i < message_count; i++) {
        mvwprintw(chat_win, i + 1, 1, "%s", display_lines[i]);
    }

    wrefresh(chat_win); //refreshes the chat window
}

/*
FUNCTION    : store_message
DESCRIPTION : store the incoming m essage in a buffer and chat window will also be updated
PARAMETERS  : const char *message - The message to be stored.
RETURNS     : None
*/

void store_message(const char* message) {
    pthread_mutex_lock(&clients_mutex);
    if (message_count < MAX_LINES) {
        strcpy(last_messages[message_count], message); //store the message
        message_count++;
    }
    else {
        for (int i = 1; i < MAX_LINES; i++) {
            strcpy(last_messages[i - 1], last_messages[i]);
        }
        strcpy(last_messages[MAX_LINES - 1], message);
    }
    update_chat_window(message); //updates the chat window with the new message
    pthread_mutex_unlock(&clients_mutex);
}

/*
FUNCTION    : broadcast_message
DESCRIPTION : send the message to all user
PARAMETERS  : const char *message - The message to broadcast.
              int sender_sock - The socket of the sender to exclude from broadcasting.
RETURN : none
*/
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

/*
FUNCTION    : remove_client
DESCRIPTION : Removes gthe client from the list and send into all other users.
PARAMETERS  : Client *client - A pointer to the client to be removed.
RETURNS     : None
*/
void remove_client(Client* client) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sockfd == client->sockfd) {
            char disconnect_msg[BUFFER_SIZE];
            snprintf(disconnect_msg, BUFFER_SIZE, "%s has left the chat", client->username);
            broadcast_message(disconnect_msg, client->sockfd);


            close(clients[i].sockfd); //clients socked is closed
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--; //client cpunt is decreased
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/*
FUNCTION    : handle_client
DESCRIPTION : handles communication with a single client
PARAMETERS  : void *arg - A pointer to the client data.
RETURNS     : None
*/
void* handle_client(void* arg) {
    Client* client = (Client*)arg;
    char buffer[BUFFER_SIZE]; //bufeer for recieving messages
    char message[BUFFER_SIZE];
    int bytes_received;
    char client_ip[INET_ADDRSTRLEN];

    // getting client ip
    inet_ntop(AF_INET, &(client->address.sin_addr), client_ip, INET_ADDRSTRLEN);

    // receiving username
    bytes_received = recv(client->sockfd, client->username, 50, 0);
    if (bytes_received <= 0) {
        remove_client(client);
        return NULL;
    }
    client->username[bytes_received] = '\0';

    //alerts other about clients has joined
    snprintf(message, BUFFER_SIZE, "%s [%s] has joined the chat",
        client->username, client_ip);
    broadcast_message(message, client->sockfd);

    while (server_running) { //handles incoming messages
        bytes_received = recv(client->sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            break; //if error it will break the loop
        }
        buffer[bytes_received] = '\0';

        //checks for disconnection command and exits
        if (strcmp(buffer, ">>bye<<") == 0) {

            snprintf(message, BUFFER_SIZE, "%s [%s] has left the chat",
                client->username, client_ip);
            broadcast_message(message, client->sockfd);


            usleep(100000);

            break;
        }


        size_t prefix_len = strlen(client->username) + strlen(client_ip) + 5;
        size_t max_msg_len = BUFFER_SIZE - prefix_len - 1; //length

        if (strlen(buffer) > max_msg_len) {
            buffer[max_msg_len] = '\0';
        }

        //formats the message
        snprintf(message, BUFFER_SIZE, "%s [%s]: %.*s",
            client->username, client_ip, (int)max_msg_len, buffer);
        broadcast_message(message, client->sockfd);//and then sends it
    }

    remove_client(client); //removes client
    return NULL;
}
