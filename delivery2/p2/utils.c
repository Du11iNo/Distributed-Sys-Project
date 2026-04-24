#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* FOR THE PEER: Call IMMEDIATELY AFTER successfully sending the PONG */
void log_pong_sent(const char *nick) {
    // Only prints the message if the DEBUG_PONG environment variable exists
    // to avoid interfering too much with the user experience
    if (getenv("DEBUG_PONG")) {
        time_t now = time(NULL);
        printf("\n[%ld] [PONG SENT] nick: %s\n%s> ", (long)now, nick, nick);
        fflush(stdout);
    }
}

/* FOR THE BROKER: Call IMMEDIATELY AFTER receiving and processing a PONG */
void log_pong_received(const char *nick) {
    time_t now = time(NULL);
    printf("[%ld] [PONG RECEIVED] nick: %s\n", (long)now, nick);
    fflush(stdout);
}

/* FOR THE BROKER: Call IMMEDIATELY AFTER removing a user due to timeout */
void log_peer_evicted(const char *nick) {
    time_t now = time(NULL);
    printf("[%ld] [PEER EVICTED] nick: %s evicted from registry\n", (long)now,
           nick);
    fflush(stdout);
}
