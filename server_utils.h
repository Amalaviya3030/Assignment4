/*
FILE          : server_utils.h
PROJECT       : Chat Server
PROGRAMMER    : Anchita Kakroa, Uttam Arora, Aryan Malviya
FIRST VERSION : 31 March 2025
DESCRIPTION   : header file for chat server whill will contain neccessary function protypes and client connection
*/

#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <ncurses.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_LINES 100

typedef struct {
    struct sockaddr_in address;
    int sockfd;
    char username[50];
    pthread_t thread;
} Client;

// global variables
extern Client clients[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;
extern WINDOW* chat_win, * input_win;
extern char last_messages[MAX_LINES][BUFFER_SIZE];
extern int message_count;
extern volatile sig_atomic_t server_running;

// functions
void init_ncurses(void);
void update_chat_window(const char* message);
void store_message(const char* message);
void broadcast_message(const char* message, int sender_sock);
void* handle_client(void* arg);
void handle_signal(int sig);
void cleanup_server(void);

#endif 
