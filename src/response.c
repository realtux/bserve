#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "response.h"

thun_response *thun_init_response(void) {
    thun_response *response = malloc(sizeof(thun_response));
    response->body = malloc(1 * sizeof(char));
    *response->body = '\0';

    return response;
}

char *init_response_string(int code) {
    char *http = "HTTP/1.1";

    char buffer[32];
    if (code == STATUS_SUCCESS_OK)
        strcpy(buffer, "OK");

    int res_strlen = strlen(http) + 1 + 3 + 1 + strlen(buffer) + 3;

    char *response_string = malloc(res_strlen * sizeof(char));
    snprintf(response_string, res_strlen, "%s %d %s\r\n", http, code, buffer);

    return response_string;
}

void append_txt_header(char *res_string, const char *header, const char *value) {
    int new_len = strlen(res_string) + strlen(header) + strlen(value) + 2 + 1;

    res_string = realloc(res_string, new_len);

    strcat(res_string, header);
    strcat(res_string, ": ");
    strcat(res_string, value);
    strcat(res_string, "\r\n");
}

void append_int_header(char *res_string, const char *header, int value) {
    char buf[16];
    sprintf(buf, "%d", value);

    append_txt_header(res_string, header, buf);
}

void append_body(char *res_string, const char *body) {
    int new_len = strlen(res_string) + strlen(body) + 2 + 1;

    res_string = realloc(res_string, new_len);

    strcat(res_string, "\r\n");
    strcat(res_string, body);
}

void thun_send_response_200(thun_response *res) {
    char *res_string = init_response_string(STATUS_SUCCESS_OK);

    append_int_header(res_string, "content-length", strlen(res->body));
    append_txt_header(res_string, "content-type", "text/html");

    append_body(res_string, res->body);

    int r = write(res->socket, res_string, strlen(res_string));

    if (r < 0)
        thunder_fatal("failed to write to socket");
}
