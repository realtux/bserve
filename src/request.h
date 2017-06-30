#ifndef REQUEST_H
#define REQUEST_H

#include "openssl/ssl.h"

typedef struct {
    char *key;
    char *value;
} req_header;

typedef struct {
    int count;
    req_header *headers;
} req_headers;

typedef struct {
    char *method;
    char *url;
    char *path;
    char *version;
    char *body;
    req_headers *header_set;
} bs_request;

typedef struct {
    int fd;
    int is_ssl;
    SSL *ssl;
} req_meta;

void *accept_request(void*);

#endif
