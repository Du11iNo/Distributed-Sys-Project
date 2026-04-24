#ifndef __UTIL_H__

#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <string.h>

#define FALSE 0
#define TRUE 1
#define MSG_SIZE 256
#define LINE_SIZE 512
#define MAX_RESPONSE_SIZE 256
#define FILENAME_SIZE 128
#define DIR_SIZE 256
#define CMD_SIZE 20

int max(int a, int b);
int validate_number(char *str);
int validate_ip(char *ip);
double randRange(double min, double max);
void log_debug(char *msg);

#define __UTIL_H__
#endif
