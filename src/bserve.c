#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "openssl/ssl.h"
#include "openssl/err.h"

#include "config.h"
#include "error.h"
#include "ipc.h"
#include "request.h"

extern bs_config config;

int sock_fd;
extern int ipc_sock_fd;

void term(int signo) {
    printf("received sigterm[%d], killing\n", signo);
    close(sock_fd);
    close(ipc_sock_fd);
    exit(0);
}

void *start_http(void *arg) {
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    pthread_t thread;
    pthread_attr_t thread_attr;
    int bind_res;

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0)
        bs_fatal("opening socket failed");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(config.host_selected ? config.host : "127.0.0.1");
    server_addr.sin_port = htons(config.port_selected ? atoi(config.port) : 8080);

    bind_res = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (bind_res < 0)
        bs_fatal("address binding failed");

    listen(sock_fd, 5);

    for (;;) {
        req_meta *meta;
        int conn_fd;

        conn_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_size);

        if (conn_fd < 0)
            bs_fatal("connection accept failed");

        meta = malloc(sizeof(req_meta));
        meta->fd = conn_fd;
        meta->is_ssl = 0;

        pthread_create(&thread, &thread_attr, accept_request, meta);
    }

    return NULL;
}

void *start_https(void *arg) {
    SSL_CTX *ctx;
    SSL_METHOD *method;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    pthread_t thread;
    pthread_attr_t thread_attr;
    int bind_res;

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    method = TLSv1_2_server_method();
    ctx = SSL_CTX_new(method);

    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (SSL_CTX_use_certificate_file(ctx, "mycert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "mycert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0)
        bs_fatal("opening socket failed");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(config.host_selected ? config.host : "127.0.0.1");
    server_addr.sin_port = htons(config.port_selected ? atoi(config.port) : 8080);

    bind_res = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (bind_res < 0)
        bs_fatal("address binding failed");

    listen(sock_fd, 5);

    for (;;) {
        req_meta *meta;
        int conn_fd;

        conn_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_size);

        if (conn_fd < 0)
            bs_fatal("connection accept failed");

        meta = malloc(sizeof(req_meta));
        meta->fd = conn_fd;
        meta->is_ssl = 1;
        meta->ssl = SSL_new(ctx);

        SSL_set_fd(meta->ssl, meta->fd);

        pthread_create(&thread, &thread_attr, accept_request, meta);
    }

    return NULL;
}

int main(int argc, char **argv) {
    pthread_t ipc_thread, http_thread, https_thread;
    pthread_attr_t thread_attr;

    bs_config_parse_opts(argc, argv);

    signal(SIGTERM, term);
    signal(SIGINT, term);

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    /**
     * start listening on the ipc channel so other processes
     * started by bserve can request their params
     */
    pthread_create(&ipc_thread, &thread_attr, bs_start_ipc, NULL);

    /*pthread_create(&http_thread, NULL, start_http, NULL);*/
    pthread_create(&https_thread, NULL, start_https, NULL);

    /*pthread_join(http_thread, NULL);*/
    pthread_join(https_thread, NULL);

    return 0;
}
