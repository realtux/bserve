#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "error.h"
#include "request.h"
#include "response.h"

void *accept_request(void *arg) {
    int conn_fd = *((int *)arg);

    free(arg);

    int len = 0;

    //char *c = malloc(1 * sizeof(char));
    char c[1];
    char *method = malloc(1 * sizeof(char));
    char *path = malloc(1 * sizeof(char));
    char *version = malloc(1 * sizeof(char));
    char *body = malloc(1 * sizeof(char));

    if (c == NULL ||
        method == NULL ||
        path == NULL ||
        version == NULL ||
        body == NULL) {

        thunder_fatal("memory allocation error");
    }

    *method = '\0';
    *path = '\0';
    *version = '\0';
    *body = '\0';

    req_headers *headers;

    int bytes_read;

    // extract method
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, 0);

        if (*c == ' ' || bytes_read <= 0) {
            break;
        }

        ++len;
        method = realloc(method, (len + 1) * sizeof(char));

        if (method == NULL) {
            thunder_fatal("memory allocation error");
        }

        strncat(method, c, 1);
    }

    len = 0;

    // extract path
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, 0);

        if (*c == ' ' || bytes_read <= 0) {
            break;
        }

        ++len;
        path = realloc(path, (len + 1) * sizeof(char));

        if (path == NULL) {
            thunder_fatal("memory allocation error");
        }

        strncat(path, c, 1);
    }

    len = 0;

    // extract version
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, 0);

        if (*c == '\r' || bytes_read <= 0) {
            // eat newline
            bytes_read = recv(conn_fd, c, 1, 0);
            break;
        }

        ++len;

        version = realloc(version, (len + 1) * sizeof(char));

        if (version == NULL) {
            thunder_fatal("memory allocation error");
        }

        strncat(version, c, 1);
    }

    // extract body if necessary
    if (strcmp(method, "GET") != 0) {

    }

    printf("method: %s\n", method);
    printf("path: %s\n", path);
    printf("version: %s\n", version);

    thun_response *response = thun_init_response();
    response->socket = conn_fd;
    response->body = realloc(response->body, sizeof(char) * 15);
    strcpy(response->body, "<h1>hello</h1>");

    thun_send_response_200(response);

    close(conn_fd);

    return NULL;
}
