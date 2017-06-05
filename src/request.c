#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "error.h"
#include "request.h"
#include "response.h"

req_headers *thun_init_headers(void) {
    req_headers *headers = malloc(1 * sizeof(req_headers));

    if (headers == NULL) {
        thunder_fatal("memory allocation error");
    }

    headers->count = 0;
    headers->headers = NULL;

    return headers;
}

req_header *thun_init_header(req_headers *headers) {
    headers->count++;
    headers->headers = realloc(headers->headers, headers->count * sizeof(req_header));

    req_header *header = &headers->headers[headers->count-1];

    header->key = malloc(1 * sizeof(char));
    header->value = malloc(1 * sizeof(char));

    *header->key = '\0';
    *header->value = '\0';

    return header;
}

void thun_dealloc_headers(req_headers *headers) {
    int i;

    for (i = 0; i < headers->count; ++i) {
        free(headers->headers[i].key);
        free(headers->headers[i].value);
    }

    if (headers->headers != NULL) free(headers->headers);
    free(headers);
}

int get_content_length(req_headers *headers) {
    int i;

    for (i = 0; i < headers->count; ++i) {
        if (strcmp(headers->headers[i].key, "Content-Length") == 0) {
            return atoi(headers->headers[i].value);
        }
    }

    return 0;
}

char *get_path(const char *url) {
    char *path = malloc(1 * sizeof(char));
    *path = '\0';

    int i;
    int path_len = 0;

    for (i = 0; i < (int)strlen(url); ++i) {
        if (i == 0 && url[i] == '/') continue;
        if (url[i] == '?') break;

        ++path_len;
        path = realloc(path, (path_len + 1) * sizeof(char));
        strncat(path, url + i, 1);
    }

    return path;
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
    char *url = malloc(1 * sizeof(char));
    char *version = malloc(1 * sizeof(char));
    char *body = malloc(1 * sizeof(char));

    if (method == NULL || url == NULL || version == NULL || body == NULL) {
        thunder_fatal("memory allocation error");
    }

    *method = '\0';
    *url = '\0';
    *version = '\0';
    *body = '\0';

    req_headers *headers = thun_init_headers();

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

    // extract url
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, 0);

        if (*c == ' ' || bytes_read <= 0) {
            break;
        }

        ++len;
        url = realloc(url, (len + 1) * sizeof(char));

        if (url == NULL) {
            thunder_fatal("memory allocation error");
        }

        strncat(url, c, 1);
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

        // create new header and extract
        req_header *header = thun_init_header(headers);

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
    // printf("url: %s\n", url);
    // printf("version: %s\n", version);
    // dump_headers(headers);
    // printf("body: %s\n", body);

    thun_response *response = thun_init_response();
    response->socket = conn_fd;

    // do something with request
    char *path = get_path(url);
    printf("%s /%s\n", method, path);

    if (strcmp(path, "") != 0) {
        FILE *fp = fopen(path, "rb");

        if (fp == NULL) {
            response->body = realloc(response->body, 15 * sizeof(char));
            response->length = 14;
            strcpy(response->body, "file not found");

            thun_send_response_200(response);
        } else {
            int ch;
            int ch_read = 0;
            while ((ch = fgetc(fp)) != EOF) {
                ++ch_read;
                response->body = realloc(response->body, (ch_read + 1) * sizeof(char));
                response->body[ch_read-1] = ch;
            }

            fclose(fp);

            response->body[ch_read] = '\0';
            response->length = ch_read;

            thun_send_response_200(response);
        }
    } else {
        printf("falling back to rules\n");

        response->body = realloc(response->body, 15 * sizeof(char));
        response->length = 14;
        strcpy(response->body, "file not found");

        thun_send_response_200(response);
    }

    shutdown(conn_fd, SHUT_WR);

    // eat res
    while (recv(conn_fd, c, 1, 0) > 0);

    close(conn_fd);

    // cleanup request
    free(path);
    free(method);
    free(url);
    free(version);
    free(body);
    thun_dealloc_headers(headers);

    // cleanup response
    thun_dealloc_response(response);

    return NULL;
}
