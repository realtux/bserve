#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"

#define STATUS_INFORMATIONAL_CONTINUE 100
#define STATUS_INFORMATIONAL_SWITCHING_PROTOCOLS 101
#define STATUS_INFORMATIONAL_PROCESSING 102
#define STATUS_SUCCESS_OK 200
#define STATUS_SUCCESS_CREATED 201
#define STATUS_SUCCESS_ACCEPTED 202
#define STATUS_SUCCESS_NON_AUTHORITATIVE_INFORMATION 203
#define STATUS_SUCCESS_NO_CONTENT 204
#define STATUS_SUCCESS_RESET_CONTENT 205
#define STATUS_SUCCESS_PARTIAL_CONTENT 206
#define STATUS_SUCCESS_MULTI_STATUS 207
#define STATUS_SUCCESS_ALREADY_REPORTED 208
#define STATUS_SUCCESS_IM_USED 226
#define STATUS_REDIRECTION_MULTIPLE_CHOICES 300
#define STATUS_REDIRECTION_MOVED_PERMANENTLY 301
#define STATUS_REDIRECTION_FOUND 302
#define STATUS_REDIRECTION_SEE_OTHER 303
#define STATUS_REDIRECTION_NOT_MODIFIED 304
#define STATUS_REDIRECTION_USE_PROXY 305
#define STATUS_REDIRECTION_TEMPORARY_REDIRECT 307
#define STATUS_REDIRECTION_PERMANENT_REDIRECT 308
#define STATUS_CLIENT_ERROR_BAD_REQUEST 400
#define STATUS_CLIENT_ERROR_UNAUTHORIZED 401
#define STATUS_CLIENT_ERROR_PAYMENT_REQUIRED 402
#define STATUS_CLIENT_ERROR_FORBIDDEN 403
#define STATUS_CLIENT_ERROR_NOT_FOUND 404
#define STATUS_CLIENT_ERROR_METHOD_NOT_ALLOWED 405
#define STATUS_CLIENT_ERROR_NOT_ACCEPTABLE 406
#define STATUS_CLIENT_ERROR_PROXY_AUTHENTICATION_REQUIRED 407
#define STATUS_CLIENT_ERROR_REQUEST_TIMEOUT 408
#define STATUS_CLIENT_ERROR_CONFLICT 409
#define STATUS_CLIENT_ERROR_GONE 410
#define STATUS_CLIENT_ERROR_LENGTH_REQUIRED 411
#define STATUS_CLIENT_ERROR_PRECONDITION_FAILED 412
#define STATUS_CLIENT_ERROR_PAYLOAD_TOO_LARGE 413
#define STATUS_CLIENT_ERROR_REQUEST_URI_TOO_LONG 414
#define STATUS_CLIENT_ERROR_UNSUPPORTED_MEDIA_TYPE 415
#define STATUS_CLIENT_ERROR_REQUESTED_RANGE_NOT_SATISFIABLE 416
#define STATUS_CLIENT_ERROR_EXPECTATION_FAILED 417
#define STATUS_CLIENT_ERROR_IM_A_TEAPOT 418
#define STATUS_CLIENT_ERROR_MISDIRECTED_REQUEST 421
#define STATUS_CLIENT_ERROR_UNPROCESSABLE_ENTITY 422
#define STATUS_CLIENT_ERROR_LOCKED 423
#define STATUS_CLIENT_ERROR_FAILED_DEPENDENCY 424
#define STATUS_CLIENT_ERROR_UPGRADE_REQUIRED 426
#define STATUS_CLIENT_ERROR_PRECONDITION_REQUIRED 428
#define STATUS_CLIENT_ERROR_TOO_MANY_REQUESTS 429
#define STATUS_CLIENT_ERROR_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define STATUS_CLIENT_ERROR_CONNECTION_CLOSED_WITHOUT_RESPONSE 444
#define STATUS_CLIENT_ERROR_UNAVAILABLE_FOR_LEGAL_REASONS 451
#define STATUS_CLIENT_ERROR_CLIENT_CLOSED_REQUEST 499
#define STATUS_SERVER_ERROR_INTERNAL_SERVER_ERROR 500
#define STATUS_SERVER_ERROR_NOT_IMPLEMENTED 501
#define STATUS_SERVER_ERROR_BAD_GATEWAY 502
#define STATUS_SERVER_ERROR_SERVICE_UNAVAILABLE 503
#define STATUS_SERVER_ERROR_GATEWAY_TIMEOUT 504
#define STATUS_SERVER_ERROR_HTTP_VERSION_NOT_SUPPORTED 505
#define STATUS_SERVER_ERROR_VARIANT_ALSO_NEGOTIATES 506
#define STATUS_SERVER_ERROR_INSUFFICIENT_STORAGE 507
#define STATUS_SERVER_ERROR_LOOP_DETECTED 508
#define STATUS_SERVER_ERROR_NOT_EXTENDED 510
#define STATUS_SERVER_ERROR_NETWORK_AUTHENTICATION_REQUIRED 511
#define STATUS_SERVER_ERROR_NETWORK_CONNECT_TIMEOUT_ERROR 599

typedef struct {
    int socket;
    int body_len;
    char *body;
} bs_response;

bs_response *bs_init_response(void);
void bs_dealloc_response(bs_response*);
void bs_send_response(int, bs_request*, bs_response*);
void bs_transmit_data(int, const char *, int);

#endif
