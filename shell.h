#ifndef SHELL_H
#define SHELL_H

#define _BSD_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>
#include <ctype.h>

void sig_handler(int sig);
int execute(char *argv[]);
char** parse(char* line);
int* parseRedirect(char **args);
void closeFD(int fd[][2], int count);

#endif
