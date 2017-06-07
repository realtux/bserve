#ifndef RESPONSE_H
#define RESPONSE_H

#define STATUS_SUCCESS_OK 200
#define STATUS_CLIENT_ERROR_NOT_FOUND 404
#define STATUS_SERVER_ERROR_INTERNAL_SERVER_ERROR 500

typedef struct {
    int socket;
    int body_len;
    char *body;
} bs_response;

bs_response *bs_init_response(void);
void bs_dealloc_response(bs_response*);
void bs_send_response(int, bs_request*, bs_response*);

#endif
