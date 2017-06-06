#ifndef REQUEST_H
#define REQUEST_H

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

void *accept_request(void*);

#endif
