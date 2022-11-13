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
#include "state/Scene.hpp"
#include <boost/program_options.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;
namespace po = boost::program_options;

static PalmScene* SCENE = NULL;

static const float UPDATE_FREQUENCY = 1.0f / 20.0f;

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
        
        // Insert the newly found sensor data into the scene.
        if(SCENE){
            (*SCENE).handleSensorData(*sensor_data);
        }
        
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



void run_communication_thread(){
    // This thread is responsible for communicating data back and forth with the simulator
    // First, connect to the simulator on port 8888
    boost::asio::io_service io_service;
    //socket creation
    ip::tcp::socket socket(io_service);
    //connection
    socket.connect(ip::tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 8888 ));
    // request/message from client
    const std::string msg = "Hello from Client!\n";
    boost::system::error_code error;
    boost::asio::write( socket, boost::asio::buffer(msg), error );
    
 // getting response from server
    boost::asio::streambuf receive_buffer;
    boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
    if( error && error != boost::asio::error::eof ) {
        cout << "receive failed: " << error.message() << endl;
    }
    else {
        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
        cout << data << endl;
    }
    return 0;

}

int argsToConfiguration(int argc, char** argv, PalmSceneConfiguration& configuration){
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("planner", "Type of planner to use."),
        ("delta", "Type of delta function to use."),
        ("gesture", "Type of gesture function to use.")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Insert other command line logic here that we need
}

int main(int argc, char** argv) {

    // Initialize scene

    PalmSceneConfiguration config{};
    if(argsToConfiguration(argc, argv, config)){
        return -1;
    }
    SCENE = new PalmScene(config);
   
    pthread_t serverThread;
    int err;
    if(err = pthread_create(&serverThread, NULL, (void *(*)(void *))&run_server_loop, NULL)){
        fprintf(stderr, "Error creating server thread.\n");
        handle_error_en(err, "pthread_create");
    } 

    pthread_t communicatorThread;
    int err;
    if(err = pthread_create(&communicatorThread, NULL, (void *(*)(void *))&run_server_loop, NULL)){
        fprintf(stderr, "Error creating server thread.\n");
        handle_error_en(err, "pthread_create");
    } 

    printf("Server started.\n");

    printf("Press any key to exit.\n");
    getchar();

    pthread_cancel(serverThread);
    free(SCENE);

    return 0;
}