#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "error.h"

bs_config config;

void menu(void) {
    printf(
        "usage: bserve [--version] [--help] [-h source_host] [-p source_port]\n"
        "              [-r doc_root]\n"
    );
}

void bs_dump_config(void) {
    printf("config options:\n");
    printf("  host_selected: %d\n", config.host_selected);
    printf("  host: %s\n", config.host);
    printf("  port_selected: %d\n", config.port_selected);
    printf("  port: %s\n", config.port);
}

void bs_config_parse_opts(int argc, char **argv) {
    config.host_selected = 0;
    config.host = NULL;
    config.port_selected = 0;
    config.port = NULL;

    if (argc <= 1) return;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--version") == 0) {
            printf("%s\n", BSERVE_VERSION);
            exit(0);
        } else if (strcmp(argv[i], "--help") == 0) {
            menu();
            exit(0);
        } else if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 >= argc) bserve_fatal("too few arguments");

            config.host_selected = 1;
            config.host = malloc((strlen(argv[i+1]) + 1) * sizeof(char));
            strcpy(config.host, argv[i+1]);

            ++i;
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) bserve_fatal("too few arguments");

            config.port_selected = 1;
            config.port = malloc((strlen(argv[i+1]) + 1) * sizeof(char));
            strcpy(config.port, argv[i+1]);

            ++i;
        }
    }

    bs_dump_config();
}
