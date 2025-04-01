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
    time_t now;
    time(&now);
    char timestamp[26];
    ctime_r(&now, timestamp);
    timestamp[24] = '\0';

    if (line_count < MAX_LINES) {
        snprintf(chat_history[line_count], BUFFER_SIZE, "[%s] %s", timestamp, message);
        line_count++;
    }
    else {
        for (int i = 1; i < MAX_LINES; i++) {
            strcpy(chat_history[i - 1], chat_history[i]);
        }
        snprintf(chat_history[MAX_LINES - 1], BUFFER_SIZE, "[%s] %s", timestamp, message);
    }


}
