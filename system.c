#include <stdlib.h>
#include <stdio.h>

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 100
#endif

int system(const char *command);

extern int commandToBuffer(char* buffer[], char * command)

{ 
    FILE *fp;

    if ((fp = popen(command, "r")) == NULL) {
        g_print("Unable to open pipe\n");
        return -1;
    } 

    int buffer_at = 0;

    char * buf = malloc(MAX_LINE_LENGTH);
    while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL) {
        *(buf + strlen(buf) - 1) = 0;
        buffer[buffer_at++] = buf;
        buf = malloc(MAX_LINE_LENGTH);
    }

    if (pclose(fp)) {
        g_print("Command not found or exited with error status\n");
        exit(1);
        return -1;
    }

    return buffer_at;
}