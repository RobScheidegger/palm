#pragma once

#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include "../tracking/Tracking.hpp"
#pragma comment(lib, "ws2_32.lib")

const static int PROXY_PORT = 8877;

SOCKET* makeCoreSocket(char* address, int port);

void closeCoreSocket(SOCKET* socket);

void sendSensorData(SOCKET* socket, HandSensorData data);