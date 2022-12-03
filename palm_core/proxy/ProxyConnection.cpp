
#include "ProxyConnection.h"

SOCKET* makeCoreSocket(char* address, int port) {
    WSADATA wsa;
    SOCKET* s = (SOCKET*)malloc(sizeof(SOCKET));
    struct sockaddr_in server;
    char c = 0;

    printf("Attempting to open socket with address %s on port %i", address, port);

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d.", WSAGetLastError());
        return NULL;
    }

    printf("Initialised.\n");

    //Create a socket
    if ((*s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d.\n", WSAGetLastError());
        return NULL;
    }
    printf("Socket created. Connecting...\n");
    memset(&server, 0, sizeof server);
    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = port;

    //Connect to remote server
    if (connect(*s, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        printf("Connect error:%d.\nPress a key to exit...", WSAGetLastError());
        closesocket(*s);
        WSACleanup();
        return NULL;
    }

    printf("Successfully connected to %s on port %i", address, port);
    return s;
}

void closeCoreSocket(SOCKET* socket) {
    closesocket(*socket);
    free(socket);
}

void sendSensorData(SOCKET* socket, HandSensorData data) {
    if (send(*socket, (char*)(void*)&data, sizeof(HandSensorData), 0) == -1)
        exit(1);
}