#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>

#define PORT 8080
#define MAX_LINES 100
#define MAX_USERNAME 50
#define MAX_MESSAGE 1024

// messages
#define MSG_TYPE_CHAT 1
#define MSG_TYPE_JOIN 2
#define MSG_TYPE_LEAVE 3


// protocols
typedef struct {
    uint8_t type;
    char username[MAX_USERNAME];
    char message[MAX_MESSAGE];
} ChatMessage;

#endif 