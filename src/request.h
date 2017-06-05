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

void *accept_request(void*);

#endif
