#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <ncurses.h>

#define MAX_LINES 100
#define BUFFER_SIZE 1024

// ext variable
extern WINDOW* chat_win;
extern char chat_history[MAX_LINES][BUFFER_SIZE];
extern int line_count;

// functions
void update_chat_window(const char* message);
void* receive_messages(void* arg);
void* send_messages(void* arg);

#endif 
