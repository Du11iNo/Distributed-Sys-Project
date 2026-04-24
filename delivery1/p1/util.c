#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int max(int a, int b) { return a > b ? a : b; }

// Utility function that determines whether the characters of a string
// represent a numeric value
int validate_number(char *str) {
    if ((*str == '-') ||
        (*str == '+')) // if the first character is a sign, ignore it
        str++;         // move to the next character (if any)
    while (*str) {
        if (!isdigit(*str)) { // if the character is not a number, return false
            return FALSE;
        }
        str++; // move to the next character
    }
    return TRUE;
}

// Utility function that validates whether a character string represents
// a valid IPv4 address
int validate_ip(char *ip) { // checks whether the IP is valid or not
    int i, num, dots = 0;
    char *ptr;
    if (ip == NULL)
        return FALSE;
    ptr = strtok(ip, "."); // extract tokens from the string delimited by '.'
    if (ptr == NULL)
        return FALSE;
    while (ptr) {
        if (!validate_number(ptr)) // check whether the substring is a number
            return FALSE;
        num = atoi(ptr); // convert substring to integer
        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, "."); // extract the next substring
            if (ptr != NULL)
                dots++; // increment the delimiter counter
        } else
            return FALSE;
    }
    if (dots != 3) // if the number of '.' is not 3, return false
        return FALSE;
    return TRUE;
}

// Utility function to generate random times between a
// minimum and a maximum
double randRange(double min, double max) {
    return min + (rand() / (double)RAND_MAX * (max - min + 1));
}

// Utility function for debugging. Prints the message
// passed as a parameter, but prefixes it with a timestamp
// so the output can be ordered if it appears interleaved.
//
// Example usage:
//
//  log_debug("Message to display on screen")
//
void log_debug(char *msg) {
    struct timespec t;
    clock_gettime(_POSIX_MONOTONIC_CLOCK, &t);

    printf("[%ld.%09ld] %s", t.tv_sec, t.tv_nsec, msg);
}
