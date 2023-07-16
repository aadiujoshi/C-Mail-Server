// SINGLE CLIENT HANDLING 
// SINGLE CLIENT HANDLING 
// SINGLE CLIENT HANDLING  
// SINGLE CLIENT HANDLING 
// SINGLE CLIENT HANDLING 


#include <stdio.h>
#include <winsock2.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddress, clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    char buffer[BUFFER_SIZE] = { 0 };
    char* response = "Hello from the server!";

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        WSACleanup();
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Bind socket to a specific address and port
    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Failed to bind socket.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        printf("Failed to listen on socket.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connection
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize)) == INVALID_SOCKET) {
        printf("Failed to accept connection.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Read data from client and send a response
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    printf("Client sent: %s\n", buffer);
    send(clientSocket, response, strlen(response), 0);
    printf("Response sent.\n");

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
