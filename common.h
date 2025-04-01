/*
FILE          : common.h
PROJECT       : Chat Server
PROGRAMMER    : Anchita Kakroa, Uttam Arora, Aryan Malviya
FIRST VERSION : 31 March 2025
DESCRIPTION   : This file contains common const and lib ha=eaders for both chat server and client
*/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_error(const char* msg);

#endif 
