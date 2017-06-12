#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "request.h"
#include "response.h"

bs_response *bs_init_response(void) {
    bs_response *response = malloc(sizeof(bs_response));

    if (response == NULL) {
        bserve_fatal("memory allocation error");
    }

    response->body_len = 0;
    response->body = NULL;

    return response;
}

void bs_dealloc_response(bs_response *response) {
    if (response->body != NULL) free(response->body);
    free(response);
}

char *init_header_string(int code) {
    char *http = "HTTP/1.1";
    char buffer[32];
    int res_strlen;
    char *response_string;

    if (code == STATUS_SUCCESS_OK)
        strcpy(buffer, "OK");
    if (code == STATUS_CLIENT_ERROR_NOT_FOUND)
        strcpy(buffer, "Not Found");
    if (code == STATUS_SERVER_ERROR_INTERNAL_SERVER_ERROR)
        strcpy(buffer, "Internal Server Error");

    res_strlen = strlen(http) + 1 + 3 + 1 + strlen(buffer) + 3;

    response_string = malloc(res_strlen * sizeof(char));
    sprintf(response_string, "%s %d %s\r\n", http, code, buffer);

    return response_string;
}

void append_txt_header(char **res_string, const char *header, const char *value) {
    int new_len = strlen(*res_string) + strlen(header) + 2 + strlen(value) + 2 + 1;

    *res_string = realloc(*res_string, new_len);

    strcat(*res_string, header);
    strcat(*res_string, ": ");
    strcat(*res_string, value);
    strcat(*res_string, "\r\n");
}

void append_int_header(char **res_string, const char *header, int value) {
    char buf[16];
    sprintf(buf, "%d", value);

    append_txt_header(res_string, header, buf);
}

void terminate_headers(char **res_string) {
    int new_len = strlen(*res_string) + 2 + 1;

    *res_string = realloc(*res_string, new_len);

    strcat(*res_string, "\r\n");
}

void transmit_data(int fd, const char *data, int size) {
    int total_bytes_written = 0;

    while (size > 0) {
        int bytes_written = write(fd, data + total_bytes_written, size);

        if (bytes_written < 0)
            bserve_fatal("failed to write to socket");

        total_bytes_written += bytes_written;
        size -= bytes_written;
    }
}

void bs_send_response(int code, bs_request *request, bs_response *response) {
    char *headers = init_header_string(code);

    append_txt_header(&headers, "Server", "bserv/0.0.1");

    /* handle 404 */
    if (code == STATUS_SUCCESS_OK) {
        if (strstr(request->path, ".html")) {
            append_txt_header(&headers, "Content-Type", "text/html");
        } else if (strstr(request->path, ".js")) {
            append_txt_header(&headers, "Content-Type", "application/javascript");
        } else if (strstr(request->path, ".css")) {
            append_txt_header(&headers, "Content-Type", "text/css");
        } else if (strstr(request->path, ".jpg") || strstr(request->path, ".jpeg")) {
            append_txt_header(&headers, "Content-Type", "image/jpeg");
        } else if (strstr(request->path, ".png")) {
            append_txt_header(&headers, "Content-Type", "text/png");
        } else if (strstr(request->path, ".ico")) {
            append_txt_header(&headers, "Content-Type", "image/x-icon");
        } else {
            append_txt_header(&headers, "Content-Type", "application/octet-stream");
        }
    } else if (code == STATUS_CLIENT_ERROR_NOT_FOUND) {
        append_txt_header(&headers, "Content-Type", "text/plain");

        response->body = realloc(response->body, 20 * sizeof(char));
        response->body_len = 19;
        strcpy(response->body, "404: file not found");
    }

    append_int_header(&headers, "Content-Length", response->body_len);
    terminate_headers(&headers);

    transmit_data(response->socket, headers, strlen(headers));
    transmit_data(response->socket, response->body, response->body_len);
}
