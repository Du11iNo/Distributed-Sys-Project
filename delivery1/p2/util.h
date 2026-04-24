#ifndef __UTIL_H__

#define TRUE 1
#define FALSE 0
#define LINE_SIZE 256
#define MSG_SIZE 128
#define DIR_SIZE 256

int validate_number(char *str);
int validate_ip(char *ip);
void log_debug(char *msg);
int min(int a, int b);
unsigned char folder_exists(const char *name);

#define __UTIL_H__
#endif
