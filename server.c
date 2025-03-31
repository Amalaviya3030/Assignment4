#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    struct sockaddr_in server_addr;


    sigset_t mask, old_mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, &old_mask);


    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        return 1;
    }
}