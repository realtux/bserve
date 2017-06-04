#include <stdio.h>
#include <stdlib.h>

void thunder_fatal(const char *msg) {
    printf("thunder fatal: %s\n", msg);
    perror("from sys:");
    exit(1);
}
