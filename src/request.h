#ifndef REQUEST_H
#define REQUEST_H

typedef struct {
    char *key;
    char *value;
} header;

typedef struct {
    int count;
    header *headers;
} req_headers;

void *accept_request(void*);

#endif
