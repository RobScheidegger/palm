#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/uio.h>
#include <vector>
#include <arpa/inet.h>
#include "tracking/Tracking.hpp"
#include <errno.h>

#define handle_error_en(en, msg) \
    do {                         \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

void proxy_thread(int socket_id){
    fprintf(stderr, "Proxy thread started on socket %i\n", socket_id);
    char buffer[1024] = {0};
    while(true){
        int valread = read(socket_id, buffer, 1024);
        if(valread == -1){
            perror("read");
            exit(1);
        }

        HandSensorData* sensor_data = (HandSensorData*)malloc(sizeof(HandSensorData));
        memcpy(sensor_data, buffer, sizeof(HandSensorData));
        
        

        free(sensor_data);
    }
    
    close(socket_id);
}

void run_server_loop(){
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";

    std::vector<pthread_t> threads;
 
    // Create socket for listening
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = 8877;
 
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started, listening for connections.\n");

    while(true){
        if ((new_socket
            = accept(server_fd, (struct sockaddr*)&address,
                    (socklen_t*)&addrlen))
            < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Client %s connected", inet_ntoa(address.sin_addr));
        pthread_t thread;

        if(pthread_create(&thread, 0, (void *(*)(void *))&proxy_thread, (void*)new_socket)){
            fprintf(stderr, "Unable to create thread\n");
        }
    }

    
    
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
}

int main(int argc, char** argv) {
   
    pthread_t serverThread;
    int err;
    if(err = pthread_create(&serverThread, NULL, (void *(*)(void *))&run_server_loop, NULL)){
        fprintf(stderr, "Error creating server thread.\n");
        handle_error_en(err, "pthread_create");
    } 


    printf("Server started.\n");

    printf("Press any key to exit.\n");
    getchar();

    return 0;
}