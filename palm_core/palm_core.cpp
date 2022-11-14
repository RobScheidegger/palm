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
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;

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
        //if(SCENE){
        //    (*SCENE).handleSensorData(*sensor_data);
        //}
        
        free(sensor_data);
    }
    
    close(socket_id);
}

void run_server_loop(){
    long new_socket;
    int server_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };

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

    printf("[palm_core::server] Server started, listening for connections.\n");

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

struct CommunicationThreadParameters{
    PalmScene* scene;
};

void run_communication_thread(CommunicationThreadParameters* threadParameters){
    // This thread is responsible for communicating data back and forth with the simulator
    // First, connect to the simulator on port 8888
    PalmScene* scene = threadParameters->scene;
    fprintf(stderr, "Scene: %i, Param: %i", scene, threadParameters);
    boost::asio::io_service io_service;
    
    ip::tcp::socket socket(io_service);
    fprintf(stderr, "[palm_core::communication] Trying to open a socket connection to the sim.\n");
    
    socket.connect(ip::tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 8888 ));
    fprintf(stderr, "[palm_core::communication] Established socket connection to the sim.\n");

    while(true){
        std::string requestMessage = "R";
        boost::system::error_code error;
        boost::asio::write(socket, boost::asio::buffer(requestMessage), error);
        // getting response from server
        boost::asio::streambuf receive_buffer;
        boost::asio::read_until(socket, receive_buffer, "\n", error);

        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
        fprintf(stderr, "[palm_core::communication] Received sim data: %s\n", data);
        std::string stringData{data};
        if(stringData.length() < 4)
            continue;
        std::vector<std::string> robotStrings;
        boost::split(robotStrings, stringData, boost::is_any_of("|"));

        std::vector<RobotState> robots;

        for(std::string robotString : robotStrings){
            std::vector<std::string> coordinates;
            boost::split(coordinates, robotString, boost::is_any_of(","));

            robots.push_back(RobotState{glm::vec3{
                std::stof(coordinates[0]), 
                std::stof(coordinates[1]), 
                std::stof(coordinates[2]), 
            }});
        }
        fprintf(stderr, "[palm_core::communication] State processing sim data.\n");
        ActualRobotState nextState = (*scene).handleReceiveRobotState(ActualRobotState{robots});
        std::vector<std::string> returnRobotStrings;
        fprintf(stderr, "[palm_core::communication] Processed sim data. Constructing response string.\n");
        fprintf(stderr, "Robot size: %lu\n", nextState.robots.size());
        for(size_t i = 0; i < nextState.robots.size(); i++){
            RobotState robotState = nextState.robots[i];
            returnRobotStrings.push_back(
                std::to_string(robotState.position.x) + "," 
                + std::to_string(robotState.position.y) + "," 
                + std::to_string(robotState.position.z));
        }

        std::string responseMessage = boost::algorithm::join(returnRobotStrings, "|");
        fprintf(stderr, "[palm_core::communication] Constructed response string.\n");
        boost::asio::write(socket, boost::asio::buffer(responseMessage), error);

        sleep(UPDATE_FREQUENCY);
    }
}

int argsToConfiguration(int argc, char** argv, PalmSceneConfiguration& configuration){
    // Declare the supported options.
    /*
    po::options_description desc("Allowed options");
    desc.add_options()
        ("planner", "Type of planner to use."),
        ("delta", "Type of delta function to use."),
        ("gesture", "Type of gesture function to use.")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    */
    // Insert other command line logic here that we need
    return 0;
}

int main(int argc, char** argv) {

    // Initialize scene

    PalmSceneConfiguration config{};
    if(argsToConfiguration(argc, argv, config)){
        return -1;
    }
    PalmScene* scene = new PalmScene(config);
   
    pthread_t serverThread;
    int err;
    if(err = pthread_create(&serverThread, NULL, (void *(*)(void *))&run_server_loop, NULL)){
        fprintf(stderr, "Error creating server thread.\n");
        handle_error_en(err, "pthread_create");
    } 

    pthread_t communicatorThread;
    CommunicationThreadParameters* communicatorParameters = (CommunicationThreadParameters*)malloc(sizeof(CommunicationThreadParameters));
    communicatorParameters->scene = scene;
    if(err = pthread_create(&communicatorThread, NULL, (void *(*)(void *))&run_communication_thread, communicatorParameters)){
        fprintf(stderr, "Error creating communicator thread.\n");
        handle_error_en(err, "pthread_create");
    } 

    printf("[palm_core::main] Server started.\n");

    printf("[palm_code::main] Press any key to exit.\n");
    getchar();

    pthread_cancel(serverThread);
    free(scene);

    return 0;
}