#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error.h"
#include "request.h"
#include "response.h"

req_header *bs_init_header(req_headers *headers) {
    req_header *header;

    headers->count++;
    headers->headers = realloc(headers->headers, headers->count * sizeof(req_header));

    header = &headers->headers[headers->count-1];

    header->key = malloc(1 * sizeof(char));
    header->value = malloc(1 * sizeof(char));

    *header->key = '\0';
    *header->value = '\0';

    return header;
}

req_headers *bs_init_headers(void) {
    req_headers *headers = malloc(1 * sizeof(req_headers));

    if (headers == NULL) {
        bs_fatal("memory allocation error");
    }

    headers->count = 0;
    headers->headers = NULL;

    return headers;
}

bs_request *bs_init_request(void) {
    bs_request *request = malloc(1 * sizeof(bs_request));

    if (request == NULL) {
        bs_fatal("memory allocation error");
    }

    request->method = malloc(1 * sizeof(char));
    request->url = malloc(1 * sizeof(char));
    request->path = malloc(1 * sizeof(char));
    request->version = malloc(1 * sizeof(char));
    request->body = malloc(1 * sizeof(char));

    *request->method = '\0';
    *request->url = '\0';
    *request->path = '\0';
    *request->version = '\0';
    *request->body = '\0';

    request->header_set = bs_init_headers();

    return request;
}

void bs_dealloc_request(bs_request *request) {
    int i;

    if (request->header_set != NULL) {
        for (i = 0; i < request->header_set->count; ++i) {
            free(request->header_set->headers[i].key);
            free(request->header_set->headers[i].value);
        }

        if (request->header_set->headers != NULL) free(request->header_set->headers);
    }

    free(request->method);
    free(request->url);
    free(request->path);
    free(request->version);
    free(request->body);
    free(request->header_set);
    free(request);
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
    int i;
    int path_len = 0;
    char *path = malloc(1 * sizeof(char));

    *path = '\0';

    for (i = 0; i < (int)strlen(url); ++i) {
        if (i == 0 && url[i] == '/') continue;
        if (url[i] == '?') break;

        ++path_len;
        path = realloc(path, (path_len + 1) * sizeof(char));
        strncat(path, url + i, 1);
    }

    return path;
}

void dump_headers(bs_request *request) {
    int i;

    printf("headers:\n");
    for (i = 0; i < request->header_set->count; ++i) {
        printf("  %s: %s\n",
            request->header_set->headers[i].key,
            request->header_set->headers[i].value);
    }
}

void *accept_request(void *arg) {
    int conn_fd = *((int *)arg);
    int len = 0;
    int bytes_read;
    char c[2];
    char *path;
    bs_request *request;
    req_header *header;
    bs_response *response;
    int content_length;
    int ch;
    int ch_read = 0;
    char command[256];
    FILE *fp;
    FILE *syscom;

    free(arg);

    c[1] = '\0';

    request = bs_init_request();

    /* extract method */
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, 0);

        if (*c == ' ' || bytes_read <= 0) {
            break;
        }

        ++len;
        request->method = realloc(request->method, (len + 1) * sizeof(char));

        if (request->method == NULL) {
            bs_fatal("memory allocation error");
        }

        strncat(request->method, c, 1);
    }

    len = 0;

    /* extract url */
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, 0);

        if (*c == ' ' || bytes_read <= 0) {
            break;
        }

        ++len;
        request->url = realloc(request->url, (len + 1) * sizeof(char));

        if (request->url == NULL) {
            bs_fatal("memory allocation error");
        }

        strncat(request->url, c, 1);
    }

    len = 0;

    /* extract version */
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, 0);

        if (*c == '\r' || bytes_read <= 0) {
            /* eat newline */
            bytes_read = recv(conn_fd, c, 1, 0);
            break;
        }

        ++len;

        request->version = realloc(request->version, (len + 1) * sizeof(char));

        if (request->version == NULL) {
            bs_fatal("memory allocation error");
        }

        strncat(request->version, c, 1);
    }

    len = 0;

    /* extract headers */
    for (;;) {
        bytes_read = recv(conn_fd, c, 1, MSG_PEEK);

        if (*c == '\r' || bytes_read <= 0) {
            /* response done reading */
            break;
        }

        /* create new header and extract */
        header = bs_init_header(request->header_set);

        /* extract header key */
        len = 0;

        for (;;) {
            bytes_read = recv(conn_fd, c, 1, 0);

            if (*c == ':' || bytes_read <= 0) {
                /* eat space */
                bytes_read = recv(conn_fd, c, 1, 0);
                break;
            }

            ++len;

            header->key = realloc(header->key, (len + 1) * sizeof(char));

            strncat(header->key, c, 1);
        }

        len = 0;

        /* extract header value */
        for (;;) {
            bytes_read = recv(conn_fd, c, 1, 0);

            if (*c == '\r' || bytes_read <= 0) {
                /* eat newline */
                bytes_read = recv(conn_fd, c, 1, 0);
                break;
            }

            ++len;

            header->value = realloc(header->value, (len + 1) * sizeof(char));

            strncat(header->value, c, 1);
        }
    }

    content_length = get_content_length(request->header_set);

    /* extract body if necessary */
    if (content_length > 0 && strcmp(request->method, "GET") != 0) {
        /* eat crlf */
        bytes_read = recv(conn_fd, c, 1, 0);
        bytes_read = recv(conn_fd, c, 1, 0);

        len = 0;

        while (len < content_length) {
            bytes_read = recv(conn_fd, c, 1, 0);

            ++len;

            request->body = realloc(request->body, (len + 1) * sizeof(char));

            if (request->body == NULL) {
                bs_fatal("memory allocation error");
            }

            strncat(request->body, c, 1);
        }
    }

    response = bs_init_response();
    response->socket = conn_fd;

    path = get_path(request->url);
    request->path = realloc(request->path, (strlen(path) + 1) * sizeof(char));
    strcpy(request->path, path);
    free(path);

    if (strcmp(request->method, "") == 0) {
        printf("invalid request, tearing down\n");
        goto teardown_conn;
    }

    printf("%s /%s\n", request->method, request->path);

    /*printf("method: %s\n", request->method);
    printf("url: %s\n", request->url);
    printf("version: %s\n", request->version);
    dump_headers(request);
    printf("body: %s\n", request->body);*/

    if (strcmp(request->path, "") == 0) {
        request->path = realloc(request->path, 11 * sizeof(char));
        strcpy(request->path, "index.html");
    }

    if (0) {
        strcpy(command, "/usr/bin/python ./sample_apps/python/bootstrap.py 2>&1");
        syscom = popen(command, "r");
        while ((ch = fgetc(syscom)) != EOF) {
            ++ch_read;
            response->body = realloc(response->body, (ch_read + 1) * sizeof(char));
            response->body[ch_read-1] = ch;
        }

        pclose(syscom);

        response->body[ch_read] = '\0';
        response->body_len = ch_read;

        bs_send_response(STATUS_SUCCESS_OK, request, response);
    } else {
        fp = fopen(request->path, "rb");

        if (fp == NULL) {
            bs_send_response(STATUS_CLIENT_ERROR_NOT_FOUND, request, response);
        } else {
            while ((ch = fgetc(fp)) != EOF) {
                ++ch_read;
                response->body = realloc(response->body, (ch_read + 1) * sizeof(char));
                response->body[ch_read-1] = ch;
            }

            fclose(fp);

            response->body[ch_read] = '\0';
            response->body_len = ch_read;

            bs_send_response(STATUS_SUCCESS_OK, request, response);
        }
    }

    teardown_conn:
    shutdown(conn_fd, SHUT_WR);

    /* eat res */
    while (recv(conn_fd, c, 1, 0) > 0);

    close(conn_fd);

    /* cleanup */
    bs_dealloc_request(request);
    bs_dealloc_response(response);

    return NULL;
}
