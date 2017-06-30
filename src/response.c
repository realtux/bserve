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
        bs_fatal("memory allocation error");
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
    const char *http = "HTTP/1.1";
    char buffer[64];
    int res_strlen;
    char *response_string;

    switch (code) {
        case STATUS_INFORMATIONAL_CONTINUE:
            strcpy(buffer, "Continue");
            break;
        case STATUS_INFORMATIONAL_SWITCHING_PROTOCOLS:
            strcpy(buffer, "Switching Protocols");
            break;
        case STATUS_INFORMATIONAL_PROCESSING:
            strcpy(buffer, "Processing");
            break;
        case STATUS_SUCCESS_OK:
            strcpy(buffer, "OK");
            break;
        case STATUS_SUCCESS_CREATED:
            strcpy(buffer, "Created");
            break;
        case STATUS_SUCCESS_ACCEPTED:
            strcpy(buffer, "Accepted");
            break;
        case STATUS_SUCCESS_NON_AUTHORITATIVE_INFORMATION:
            strcpy(buffer, "Non-authoritative Information");
            break;
        case STATUS_SUCCESS_NO_CONTENT:
            strcpy(buffer, "No Content");
            break;
        case STATUS_SUCCESS_RESET_CONTENT:
            strcpy(buffer, "Reset Content");
            break;
        case STATUS_SUCCESS_PARTIAL_CONTENT:
            strcpy(buffer, "Partial Content");
            break;
        case STATUS_SUCCESS_MULTI_STATUS:
            strcpy(buffer, "Multi-Status");
            break;
        case STATUS_SUCCESS_ALREADY_REPORTED:
            strcpy(buffer, "Already Reported");
            break;
        case STATUS_SUCCESS_IM_USED:
            strcpy(buffer, "IM Used");
            break;
        case STATUS_REDIRECTION_MULTIPLE_CHOICES:
            strcpy(buffer, "Multiple Choices");
            break;
        case STATUS_REDIRECTION_MOVED_PERMANENTLY:
            strcpy(buffer, "Moved Permanently");
            break;
        case STATUS_REDIRECTION_FOUND:
            strcpy(buffer, "Found");
            break;
        case STATUS_REDIRECTION_SEE_OTHER:
            strcpy(buffer, "See Other");
            break;
        case STATUS_REDIRECTION_NOT_MODIFIED:
            strcpy(buffer, "Not Modified");
            break;
        case STATUS_REDIRECTION_USE_PROXY:
            strcpy(buffer, "Use Proxy");
            break;
        case STATUS_REDIRECTION_TEMPORARY_REDIRECT:
            strcpy(buffer, "Temporary Redirect");
            break;
        case STATUS_REDIRECTION_PERMANENT_REDIRECT:
            strcpy(buffer, "Permanent Redirect");
            break;
        case STATUS_CLIENT_ERROR_BAD_REQUEST:
            strcpy(buffer, "Bad Request");
            break;
        case STATUS_CLIENT_ERROR_UNAUTHORIZED:
            strcpy(buffer, "Unauthorized");
            break;
        case STATUS_CLIENT_ERROR_PAYMENT_REQUIRED:
            strcpy(buffer, "Payment Required");
            break;
        case STATUS_CLIENT_ERROR_FORBIDDEN:
            strcpy(buffer, "Forbidden");
            break;
        case STATUS_CLIENT_ERROR_NOT_FOUND:
            strcpy(buffer, "Not Found");
            break;
        case STATUS_CLIENT_ERROR_METHOD_NOT_ALLOWED:
            strcpy(buffer, "Method Not Allowed");
            break;
        case STATUS_CLIENT_ERROR_NOT_ACCEPTABLE:
            strcpy(buffer, "Not Acceptable");
            break;
        case STATUS_CLIENT_ERROR_PROXY_AUTHENTICATION_REQUIRED:
            strcpy(buffer, "Proxy Authentication Required");
            break;
        case STATUS_CLIENT_ERROR_REQUEST_TIMEOUT:
            strcpy(buffer, "Request Timeout");
            break;
        case STATUS_CLIENT_ERROR_CONFLICT:
            strcpy(buffer, "Conflict");
            break;
        case STATUS_CLIENT_ERROR_GONE:
            strcpy(buffer, "Gone");
            break;
        case STATUS_CLIENT_ERROR_LENGTH_REQUIRED:
            strcpy(buffer, "Length Required");
            break;
        case STATUS_CLIENT_ERROR_PRECONDITION_FAILED:
            strcpy(buffer, "Precondition Failed");
            break;
        case STATUS_CLIENT_ERROR_PAYLOAD_TOO_LARGE:
            strcpy(buffer, "Payload Too Large");
            break;
        case STATUS_CLIENT_ERROR_REQUEST_URI_TOO_LONG:
            strcpy(buffer, "Request-URI Too Long");
            break;
        case STATUS_CLIENT_ERROR_UNSUPPORTED_MEDIA_TYPE:
            strcpy(buffer, "Unsupported Media Type");
            break;
        case STATUS_CLIENT_ERROR_REQUESTED_RANGE_NOT_SATISFIABLE:
            strcpy(buffer, "Requested Range Not Satisfiable");
            break;
        case STATUS_CLIENT_ERROR_EXPECTATION_FAILED:
            strcpy(buffer, "Expectation Failed");
            break;
        case STATUS_CLIENT_ERROR_IM_A_TEAPOT:
            strcpy(buffer, "I'm a teapot");
            break;
        case STATUS_CLIENT_ERROR_MISDIRECTED_REQUEST:
            strcpy(buffer, "Misdirected Request");
            break;
        case STATUS_CLIENT_ERROR_UNPROCESSABLE_ENTITY:
            strcpy(buffer, "Unprocessable Entity");
            break;
        case STATUS_CLIENT_ERROR_LOCKED:
            strcpy(buffer, "Locked");
            break;
        case STATUS_CLIENT_ERROR_FAILED_DEPENDENCY:
            strcpy(buffer, "Failed Dependency");
            break;
        case STATUS_CLIENT_ERROR_UPGRADE_REQUIRED:
            strcpy(buffer, "Upgrade Required");
            break;
        case STATUS_CLIENT_ERROR_PRECONDITION_REQUIRED:
            strcpy(buffer, "Precondition Required");
            break;
        case STATUS_CLIENT_ERROR_TOO_MANY_REQUESTS:
            strcpy(buffer, "Too Many Requests");
            break;
        case STATUS_CLIENT_ERROR_REQUEST_HEADER_FIELDS_TOO_LARGE:
            strcpy(buffer, "Request Header Fields Too Large");
            break;
        case STATUS_CLIENT_ERROR_CONNECTION_CLOSED_WITHOUT_RESPONSE:
            strcpy(buffer, "Connection Closed Without Response");
            break;
        case STATUS_CLIENT_ERROR_UNAVAILABLE_FOR_LEGAL_REASONS:
            strcpy(buffer, "Unavailable For Legal Reasons");
            break;
        case STATUS_CLIENT_ERROR_CLIENT_CLOSED_REQUEST:
            strcpy(buffer, "Client Closed Request");
            break;
        case STATUS_SERVER_ERROR_INTERNAL_SERVER_ERROR:
            strcpy(buffer, "Internal Server Error");
            break;
        case STATUS_SERVER_ERROR_NOT_IMPLEMENTED:
            strcpy(buffer, "Not Implemented");
            break;
        case STATUS_SERVER_ERROR_BAD_GATEWAY:
            strcpy(buffer, "Bad Gateway");
            break;
        case STATUS_SERVER_ERROR_SERVICE_UNAVAILABLE:
            strcpy(buffer, "Service Unavailable");
            break;
        case STATUS_SERVER_ERROR_GATEWAY_TIMEOUT:
            strcpy(buffer, "Gateway Timeout");
            break;
        case STATUS_SERVER_ERROR_HTTP_VERSION_NOT_SUPPORTED:
            strcpy(buffer, "HTTP Version Not Supported");
            break;
        case STATUS_SERVER_ERROR_VARIANT_ALSO_NEGOTIATES:
            strcpy(buffer, "Variant Also Negotiates");
            break;
        case STATUS_SERVER_ERROR_INSUFFICIENT_STORAGE:
            strcpy(buffer, "Insufficient Storage");
            break;
        case STATUS_SERVER_ERROR_LOOP_DETECTED:
            strcpy(buffer, "Loop Detected");
            break;
        case STATUS_SERVER_ERROR_NOT_EXTENDED:
            strcpy(buffer, "Not Extended");
            break;
        case STATUS_SERVER_ERROR_NETWORK_AUTHENTICATION_REQUIRED:
            strcpy(buffer, "Network Authentication Required");
            break;
        case STATUS_SERVER_ERROR_NETWORK_CONNECT_TIMEOUT_ERROR:
            strcpy(buffer, "Network Connect Timeout Error");
            break;
        default:
            /**
             * it will always be one of the above
             */
            break;
    }

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

void apply_content_type(char **headers, const char *path) {
    if (strstr(path, ".html")) {
        append_txt_header(headers, "Content-Type", "text/html");
    } else if (strstr(path, ".js")) {
        append_txt_header(headers, "Content-Type", "application/javascript");
    } else if (strstr(path, ".css")) {
        append_txt_header(headers, "Content-Type", "text/css");
    } else if (strstr(path, ".jpg") || strstr(path, ".jpeg")) {
        append_txt_header(headers, "Content-Type", "image/jpeg");
    } else if (strstr(path, ".png")) {
        append_txt_header(headers, "Content-Type", "text/png");
    } else if (strstr(path, ".ico")) {
        append_txt_header(headers, "Content-Type", "image/x-icon");
    } else {
        append_txt_header(headers, "Content-Type", "application/octet-stream");
    }
}

void terminate_headers(char **res_string) {
    int new_len = strlen(*res_string) + 2 + 1;

    *res_string = realloc(*res_string, new_len);

    strcat(*res_string, "\r\n");
}

void bs_transmit_data(req_meta *meta, const char *data, int size) {
    int total_bytes_written = 0;
    int bytes_written;

    while (size > 0) {
        if (meta->is_ssl == 1) {
            bytes_written = SSL_write(meta->ssl, data + total_bytes_written, size);
        } else {
            bytes_written = write(meta->fd, data + total_bytes_written, size);
        }

        if (bytes_written < 0)
            bs_fatal("failed to write to socket");

        total_bytes_written += bytes_written;
        size -= bytes_written;
    }
}

void bs_send_response(int code, bs_request *request, bs_response *response) {
    char *headers = init_header_string(code);

    append_txt_header(&headers, "Server", "bserv/0.0.1");

    /* handle 404 */
    if (code == STATUS_SUCCESS_OK) {
        apply_content_type(&headers, request->path);
    } else if (code == STATUS_CLIENT_ERROR_NOT_FOUND) {
        append_txt_header(&headers, "Content-Type", "text/plain");

        response->body = realloc(response->body, 20 * sizeof(char));
        response->body_len = 19;
        strcpy(response->body, "404: file not found");
    }

    append_int_header(&headers, "Content-Length", response->body_len);
    terminate_headers(&headers);

    bs_transmit_data(response->meta, headers, strlen(headers));
    bs_transmit_data(response->meta, response->body, response->body_len);
}
