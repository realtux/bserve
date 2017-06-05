#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "error.h"
#include "request.h"
#include "response.h"

int get_content_length(req_headers *headers) {
    int i;

    for (i = 0; i < headers->count; ++i) {
        if (strcmp(headers->headers[i].key, "Content-Length") == 0) {
            return atoi(headers->headers[i].value);
        }
    }

    return 0;
}

void dump_headers(req_headers *headers) {
    int i;

    printf("headers:\n");
    for (i = 0; i < headers->count; ++i) {
        printf("  %s: %s\n", headers->headers[i].key, headers->headers[i].value);
    }
}

void *accept_request(void *arg) {
    int conn_fd = *((int *)arg);

    free(arg);

    int len = 0;

    char c[2]; c[1] = '\0';
    char *method = malloc(1 * sizeof(char));
    char *path = malloc(1 * sizeof(char));
    char *version = malloc(1 * sizeof(char));
    char *body = malloc(1 * sizeof(char));

    if (method == NULL || path == NULL || version == NULL || body == NULL) {
        thunder_fatal("memory allocation error");
    }

    *method = '\0';
    *path = '\0';
    *version = '\0';
    *body = '\0';

    req_headers *headers = malloc(1 * sizeof(req_headers));

    if (headers == NULL) {
        thunder_fatal("memory allocation error");
    }

    headers->count = 0;
    headers->headers = NULL;

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

    len = 0;

    // extract headers
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, MSG_PEEK);

        if (*c == '\r' || bytes_read <= 0) {
            // response done reading
            break;
        }

        // extract header
        headers->count++;
        headers->headers = realloc(headers->headers, headers->count * sizeof(req_header));

        req_header *header = &headers->headers[headers->count-1];

        header->key = malloc(1 * sizeof(char));
        header->value = malloc(1 * sizeof(char));

        *header->key = '\0';
        *header->value = '\0';

        // extract header key
        len = 0;

        for (;;) {
            bytes_read = recv(conn_fd, c, 1, 0);

            if (*c == ':' || bytes_read <= 0) {
                // eat space
                bytes_read = recv(conn_fd, c, 1, 0);
                break;
            }

            ++len;

            header->key = realloc(header->key, (len + 1) * sizeof(char));

            strncat(header->key, c, 1);
        }

        len = 0;

        // extract header value
        for (;;) {
            bytes_read = recv(conn_fd, c, 1, 0);

            if (*c == '\r' || bytes_read <= 0) {
                // eat newline
                bytes_read = recv(conn_fd, c, 1, 0);
                break;
            }

            ++len;

            header->value = realloc(header->value, (len + 1) * sizeof(char));

            strncat(header->value, c, 1);
        }
    }

    int content_length = get_content_length(headers);

    // extract body if necessary
    if (content_length > 0 && strcmp(method, "GET") != 0) {
        // eat crlf
        bytes_read = recv(conn_fd, c, 1, 0);
        bytes_read = recv(conn_fd, c, 1, 0);

        len = 0;

        while (len < content_length) {
            bytes_read = recv(conn_fd, c, 1, 0);

            ++len;

            body = realloc(body, (len + 1) * sizeof(char));

            if (body == NULL) {
                thunder_fatal("memory allocation error");
            }

            strncat(body, c, 1);
        }
    }

    // printf("method: %s\n", method);
    // printf("path: %s\n", path);
    // printf("version: %s\n", version);
    // dump_headers(headers);
    // printf("body: %s\n", body);

    // do something with request

    thun_response *response = thun_init_response();
    response->socket = conn_fd;
    response->body = realloc(response->body, sizeof(char) * 3);
    strcpy(response->body, ":)");

    thun_send_response_200(response);

    shutdown(conn_fd, SHUT_WR);

    // eat res
    while (recv(conn_fd, c, 1, 0) > 0);
printf("closing fd %d\n", conn_fd);
    close(conn_fd);

    return NULL;
}
