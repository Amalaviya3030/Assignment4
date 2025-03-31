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
