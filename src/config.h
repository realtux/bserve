#ifndef CONFIG_H
#define CONFIG_H

#define BSERVE_VERSION "0.0.1"

typedef struct {
    int host_selected;
    char *host;
    int port_selected;
    char *port;
} bs_config;

void bs_config_parse_opts(int, char**);

#endif
