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

    werase(chat_win);
    box(chat_win, 0, 0);

    int max_y, max_x;
    getmaxyx(chat_win, max_y, max_x);
    mvwprintw(chat_win, 0, (max_x - 12) / 2, " Chat Room ");

    // print message
    int current_line = 1;
    for (int i = 0; i < line_count && current_line < max_y - 1; i++) {
        char* msg = chat_history[i];
        int len = strlen(msg);
        int pos = 0;

        while (pos < len && current_line < max_y - 1) {
            int remaining = len - pos;
            int width = max_x - 2;
            int chunk = (remaining > width) ? width : remaining;

            char temp[BUFFER_SIZE];
            strncpy(temp, msg + pos, chunk);
            temp[chunk] = '\0';

            mvwprintw(chat_win, current_line++, 1, "%s", temp);
            pos += chunk;
        }
    }

    wrefresh(chat_win);
}
