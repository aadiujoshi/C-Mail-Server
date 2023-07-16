#include <stdbool.h>
#include "server.h"
#include "linked_list.h"

void start_server() {
    WSADATA wsaData;
    SOCKET serverSocket;
    SOCKADDR_IN serverAddress;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock. %i\n", WSAGetLastError());
        return 1;
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket. %i\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Failed to bind socket. %i\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    //size_t _tm = SOCKET_ACCEPT_TIMEOUT_MS;
    //int res = setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&_tm, sizeof(_tm));

    if (listen(serverSocket, MAX_BACKLOG) == SOCKET_ERROR) {
        printf("Failed to listen on socket. %i\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    HANDLE active_threads[THREAD_POOL_SIZE + 1] = { 0 };
    client_pool_thread thread_pool[THREAD_POOL_SIZE] = { 0 };

    server_context sc = { wsaData, serverSocket, serverAddress, thread_pool};

    active_threads[THREAD_POOL_SIZE] = _beginthread(handle_accept, 0, &sc);

    for (size_t i = 0; i < THREAD_POOL_SIZE; i++){
        client_pool_thread* cpt = thread_pool + i;

        cpt->client_couples = create_list(sizeof(mail_client*));
        cpt->thread = _beginthread(handle_accept, 0, &sc);
        sc.thread_id = i;

        //wait for thread init
        while (sc.thread_id != -1);
    }

    WaitForMultipleObjects(THREAD_POOL_SIZE+1, active_threads, TRUE, INFINITE);

    WSACleanup();

}

void handle_accept(void* context) {
    server_context* sc = (server_context*)context;

    size_t next_free_pool = 0;

    while (1) {
        if (!sc->running)
            break;

        SOCKET client_s = 0;
        SOCKADDR client_sa;

        client_s = accept(sc->serv_sock, &client_sa, sizeof(client_sa));
        if (client_s == INVALID_SOCKET) {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                printf("\t\taccept timeout\n");
            }
            else {
                printf("Accept failed: %d\n", WSAGetLastError());
                sc->running = false;
                break;
            }
        // valid socket
        } else {
            //send accept confirmation
            int send_res = send(client_s, SERVER_ACCEPT_CLIENT_CONFIRMATION, strlen(SERVER_ACCEPT_CLIENT_CONFIRMATION), 0);

            if (send_res != SOCKET_ERROR) {

                const char client_info[CLIENT_SEND_INFO_BYTE_SIZE] = { 0 };
                int recv_res = recv(client_s, client_info, CLIENT_SEND_INFO_BYTE_SIZE, 0);

                if (recv_res != SOCKET_ERROR) {
                    mail_client* mc = (mail_client*)malloc(sizeof(mail_client));
                    mc->clnt_a = client_sa;
                    mc->clnt_s = client_s;

                    size_t p_index = -1;
                    for (size_t i = 0; i < THREAD_POOL_SIZE; i++) {
                        if (list_index_of((sc->c_pools + i)->client_couples, is_partner, mc) != -1) {
                            p_index = i;
                            break;
                        }
                    }
                    if (p_index == -1) {
                        add_client_to_pool(sc->c_pools + next_free_pool, mc);
                    }
                    else {
                        add_client_to_pool(sc->c_pools + p_index, mc);
                    }

                    next_free_pool = (++next_free_pool) % THREAD_POOL_SIZE;
                } else {
                    //receive error
                }
            } else {
                //send error
            }
        }
    }

    _endthread();
}

void handle_client_pool(void* context) {
    server_context* sc = (server_context*)context; 

    int t_id = sc->thread_id;
    sc->thread_id = -1;

    client_pool_thread* cpt = sc->c_pools + t_id;
    linked_list* c_list = cpt->client_couples;

    FD_SET set;
    SOCKET max_s = 0;

    while (1) {
        if (!sc->running)
            break;

        FD_ZERO(&set);

        //max socket dscrpt
        for (size_t i = 0; i < c_list->length; i++) {
            mail_client* mc = *((mail_client**)list_get(c_list, i));
            max_s = max(max_s, mc->clnt_s);

            FD_SET(mc->clnt_s, &set);
        }
            
        //poll
        int ready_sockets = select(max_s + 1, &set, NULL, NULL, NULL);

        if (ready_sockets == SOCKET_ERROR) {
            printf("select error %i\n", WSAGetLastError());
        }
        else if (ready_sockets > 0) {
            for (int i = 0; i < set.fd_count; ++i) {
                SOCKET cs = set.fd_array[i];
                mail_client* mc = *((mail_client**)list_get(c_list, i));

                if (FD_ISSET(cs, &set)) {

                    char buffer[BUFFER_SIZE];

                    int bytes_recv = recv(cs, buffer, sizeof(buffer), 0);
                    if (bytes_recv == SOCKET_ERROR) {
                        printf("recv from socket error %i\n", WSAGetLastError());
                    }
                    //client gracefully disconnected
                    else if (bytes_recv == 0) {
                        free_mail_client(mc);
                        list_remove(c_list, i);
                    }
                    //client sent msg
                    else if (bytes_recv > 0){
                        int ri = list_index_of(c_list, is_partner, mc);
                        if (ri == -1) {
                            continue;
                        }
                        mail_client* r_client = *((mail_client**)list_get(c_list, ri));
                        int send_res = send(r_client->clnt_s, buffer, sizeof(buffer), 0); 
                        // ERROR CHECK THIS SEND MESSAGE FUNC
                        // ERROR CHECK THIS SEND MESSAGE FUNC
                        // ERROR CHECK THIS SEND MESSAGE FUNC
                        // ERROR CHECK THIS SEND MESSAGE FUNC
                        // ERROR CHECK THIS SEND MESSAGE FUNC
                    }
                }
            }
        }
    }

    _endthread();
}

void add_client_to_pool(client_pool_thread* cpt, mail_client* client) {
    //wait
    while (cpt->user_lock != NULL);
    cpt->user_lock = CPT_USER_LOCK_2;

    list_add(cpt->client_couples, &client);

    cpt->user_lock = CPT_USER_LOCK_1;
}

bool is_partner(mail_client** couple, mail_client** compare) {
    mail_client* cc = *couple;
    mail_client* mc = *compare;

    if (strcmp(cc->rcvr_id, mc->clnt_s) & strcmp(cc->clnt_id, mc->rcvr_id)) {
        return true;
    }
    return false;
}

void free_mail_client(mail_client* mc) {
    free(mc);
}
