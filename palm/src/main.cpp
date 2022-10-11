#include <iostream>
#include <string.h>
#include <thread>
#include "LeapC.h"
#include "main.h"

static LEAP_CONNECTION connectionHandle;
static bool _isRunning = false;
std::thread pollingThread;

void serviceMessageLoop()
{
    eLeapRS result;
    LEAP_CONNECTION_MESSAGE msg;
    while (_isRunning)
    {
        unsigned int timeout = 1000;
        result = LeapPollConnection(connectionHandle, timeout, &msg);
        // Handle message
    }
}

LEAP_CONNECTION *OpenConnection()
{
    eLeapRS result = LeapCreateConnection(NULL, &connectionHandle);
    if (result == eLeapRS_Success)
    {
        result = LeapOpenConnection(connectionHandle);
        if (result == eLeapRS_Success)
        {
            _isRunning = true;
            pthread_create(&pollingThread, NULL, serviceMessageLoop, NULL);
        }
    }
    return &connectionHandle;
}

int main(int argc, char **argv)
{
    OpenConnection();
    // Keep this process running until Enter is pressed
    std::cout
        << "Press Enter to quit..." << std::endl;
    std::cin.get();

    return 0;
}