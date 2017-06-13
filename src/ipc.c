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

#include "config.h"
#include "error.h"
#include "response.h"

extern bs_config config;

struct node {
    int fd;
    struct node *next;
};

typedef struct node node;

int ipc_sock_fd;

node *head = NULL;

node *bs_ipc_list_get(int fd) {
    node *ptr;

    if (head == NULL) {
        return NULL;
    }

    ptr = head;

    do {
        if (ptr->fd == fd) {
            return ptr;
        }

        ptr = ptr->next;
    } while (ptr != NULL);

    return NULL;
}

void bs_ipc_list_remove(int fd) {
    node *ptr;
    node *prev;

    if (head == NULL) {
        return;
    }

    ptr = head;

    if (ptr->fd == fd) {
        head = ptr->next;
        free(ptr);
        return;
    }

    if (ptr->next == NULL) {
        return;
    }

    for (;;) {
        prev = ptr;
        ptr = ptr->next;

        if (ptr == NULL) {
            return;
        }

        if (ptr->fd == fd) {
            if (ptr->next != NULL) {
                prev->next = ptr->next;
            } else {
                prev->next = NULL;
            }
            free(ptr);
            return;
        }
    }
}

void bs_ipc_list_insert(int fd) {
    node *ptr;
    node *link;

    if (bs_ipc_list_get(fd) != NULL) {
        bs_ipc_list_remove(fd);
    }

    link = malloc(sizeof(node));
    link->fd = fd;
    link->next = NULL;

    if (head == NULL) {
        head = link;
        return;
    }

    ptr = head;

    while (ptr->next != NULL) {
        ptr = ptr->next;
    }

    ptr->next = link;
}

void bs_dump_list(void) {
    node *ptr;

    if (head == NULL) {
        printf("list empty\n");
        return;
    }

    ptr = head;

    printf("starting ipc list dump:\n");
    do {
        printf("dumping fd: %d\n", ptr->fd);
        ptr = ptr->next;
    } while (ptr != NULL);
}

void *handle_ipc_request(void *arg) {
    int ipc_fd;
    int bytes_read;
    int len;
    char *buf;
    char *node_buf;
    int conn_fd;
    char c;
    node *ipc_node;

    conn_fd = *((int *)arg);
    free(arg);

    buf = malloc(1 * sizeof(char));

    if (buf == NULL) {
        bs_fatal("memory allocation error");
    }

    buf[0] = '0';

    len = 0;

    /**
     * requests come in the form of fd\n
     * read up to, but discard newline
     */
    for (;;) {
        bytes_read = recv(conn_fd, &c, 1, 0);

        if (c == '\n' || bytes_read <= 0) {
            break;
        }

        ++len;
        buf = realloc(buf, (len + 1) * sizeof(char));

        if (buf == NULL) {
            bs_fatal("memory allocation error");
        }

        buf[len-1] = c;
        buf[len] = '\0';
    }

    ipc_fd = atoi(buf);
    free(buf);

/**
 * replace all this garbage with a json payload of the request
 */
printf("processing for: %d\n", ipc_fd);
bs_ipc_list_insert(ipc_fd);
ipc_node = bs_ipc_list_get(ipc_fd);
node_buf = malloc(64);
sprintf(node_buf, "{\"msg\": \"test\", \"fd\": %d}", ipc_node->fd);
bs_transmit_data(conn_fd, node_buf, strlen(node_buf));
free(node_buf);

    bs_ipc_list_remove(ipc_fd);

    shutdown(conn_fd, SHUT_WR);

    /* eat res */
    while (recv(conn_fd, &c, 1, 0) > 0);

    close(conn_fd);

    return NULL;
}

void *bs_start_ipc(void) {
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int bind_res;

    pthread_t thread;
    pthread_attr_t thread_attr;

    pthread_attr_init(&thread_attr);
    /*pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);*/

    ipc_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (ipc_sock_fd < 0)
        bs_fatal("opening ipc socket failed");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8383);

    bind_res = bind(ipc_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (bind_res < 0)
        bs_fatal("ipc address binding failed");

    listen(ipc_sock_fd, 5);

    for (;;) {
        int *conn_fd = malloc(sizeof(int));

        if (conn_fd == NULL)
            bs_fatal("unable to allocate memory");

        *conn_fd = accept(ipc_sock_fd, (struct sockaddr *) &client_addr, &client_addr_size);

        if (*conn_fd < 0)
            bs_fatal("ipc connection accept failed");

        pthread_create(&thread, &thread_attr, handle_ipc_request, conn_fd);
        pthread_join(thread, NULL);
    }

    return NULL;
}
