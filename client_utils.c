#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include "../include/client_utils.h"

#define MAX_LINES 100
#define BUFFER_SIZE 1024

extern WINDOW* chat_win;
extern char chat_history[MAX_LINES][BUFFER_SIZE];
extern int line_count;

void update_chat_window(const char* message) {

}
