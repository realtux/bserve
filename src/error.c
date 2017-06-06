#include <stdio.h>
#include <stdlib.h>

void bserve_fatal(const char *msg) {
    printf("bserve fatal: %s\n", msg);
    perror("from sys:");
    exit(1);
}
