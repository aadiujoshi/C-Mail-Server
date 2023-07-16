#pragma once
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdbool.h>
#include <process.h>
#include "fwd_dec.h"
#include "linked_list.h"

#define PORT 8080
#define BUFFER_SIZE 1024

#define MAX_BACKLOG 100

#define THREAD_POOL_SIZE 3

#define SOCKET_ACCEPT_TIMEOUT_MS (0 * 1000)
#define CLIENT_POLL_INTERVAL_MS (2 * 1000)

#define CLIENT_SEND_INFO_BYTE_SIZE 100

#define SERVER_ACCEPT_CLIENT_CONFIRMATION (const char*)("s")

#define CPT_USER_LOCK_1 1
#define CPT_USER_LOCK_2 2

//#define SOCKET_EVENT unsigned int
//#define EVENT_NEW_CLIENT_CONNECTION 1
//#define EVENT_CLIENT_TERMINATE_CONNECTION 2
//#define EVENT_MESSAGE_SENT 3

//typedef struct _socket_event {
//    SOCKET_EVENT e;
//    char* raw_msg;
//} socket_event;

typedef struct _server_context {
    WSADATA info;
    SOCKET serv_sock;
    SOCKADDR_IN serv_addr;
    //count is THREAD_POOL_SIZE
    client_pool_thread* c_pools;
    volatile bool running;

    //temp field one time use on thread creation
    volatile int thread_id;
} server_context;

typedef struct _mail_client {
    SOCKADDR clnt_a;
    SOCKET clnt_s;
    char* clnt_id;
    char* rcvr_id;
} mail_client;

typedef struct _client_pool_thread {
    HANDLE thread;
    linked_list* client_couples;
    volatile int user_lock;
} client_pool_thread;

void start_server();

void handle_accept(void* context);

void handle_client_pool(void* context);

void add_client_to_pool(client_pool_thread* cpt, mail_client* client);

bool is_partner(mail_client** couple, mail_client** compare);

void free_mail_client(mail_client* mc);