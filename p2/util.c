#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "srvwww.h"
#include "util.h"

// Utility function that determines whether all characters
// in a string are numeric
int validate_number(char *str)
{
    while (*str)
    {
        if (!isdigit(*str))
        {   // if the character is not a number, return FALSE
            return FALSE;
        }
        str++; // move to the next character
    }
    return TRUE;
}

// Utility function that validates whether a string represents
// a valid IPv4 address
int validate_ip(char *ip)
{
    int num, dots = 0;
    char *copy;
    char *ptr;

    if (ip == NULL)
        return FALSE;

    copy = strdup(ip);        // to avoid modifying the original IP
    ptr = strtok(copy, ".");  // extract tokens delimited by '.'

    if (ptr == NULL)
        return FALSE;

    while (ptr)
    {
        if (!validate_number(ptr)) // check if substring is a number
            return FALSE;

        num = atoi(ptr); // convert substring to integer

        if (num >= 0 && num <= 255)
        {
            ptr = strtok(NULL, "."); // extract next substring
            if (ptr != NULL)
                dots++; // increment delimiter counter
        }
        else
            return FALSE;
    }

    if (dots != 3) // if number of '.' is not 3, return FALSE
        return FALSE;

    return TRUE;
}

// Utility function for debugging. Prints the message passed
// as parameter, but prefixes it with a timestamp so the output
// can be ordered even if printed asynchronously.
//
// Example:
//
//   log_debug("Message to print")
//
// More examples in the main program.
void log_debug(char *msg)
{
    struct timespec t;
    clock_gettime(_POSIX_MONOTONIC_CLOCK, &t);
    printf("[%ld.%09ld] %s", t.tv_sec, t.tv_nsec, msg);
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

// Function that returns TRUE if the folder name passed as argument
// exists and is a directory
unsigned char folder_exists(const char *name)
{
    struct stat buffer;
    return (stat(name, &buffer) == 0) && S_ISDIR(buffer.st_mode);
}
