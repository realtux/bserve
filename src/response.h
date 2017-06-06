#ifndef RESPONSE_H
#define RESPONSE_H

#define STATUS_SUCCESS_OK 200

typedef struct {
    int socket;
    int body_len;
    char *body;
} bs_response;

bs_response *bs_init_response(void);
void bs_dealloc_response(bs_response*);
void bs_send_response_200(bs_request*, bs_response*);

#endif
