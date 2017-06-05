#ifndef RESPONSE_H
#define RESPONSE_H

#define STATUS_SUCCESS_OK 200

typedef struct {
    int socket;
    int length;
    char *body;
} thun_response;

thun_response *thun_init_response(void);
void thun_dealloc_response(thun_response*);
void thun_send_response_200(thun_response*);

#endif
