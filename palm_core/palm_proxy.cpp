#include <stdio.h>
#include <stdlib.h>
#include "leap/LeapC.h"
#include "leap/LeapConnection.hpp"
#include "tracking/Tracking.hpp"
#include <string>
#include "proxy/ProxyConnection.h"
#include <Windows.h>
#include "sysinfoapi.h"

static LEAP_CONNECTION* connectionHandle = NULL;
static SOCKET* coreSocket = NULL;
static HANDLE recordingFile = NULL;
static long long recordingTimeStart = NULL;
static int SENSOR_DATA_FACTOR = 3;

/** Callback for when the connection opens. */
static void OnConnect(void){
  printf("Connected to Leap Tracking Service.\n");
}

/** Callback for when a device is found. */
static void OnDevice(const LEAP_DEVICE_INFO *props){
  printf("Found device %s.\n", props->serial);
}

static Position convertLeapPosition(LEAP_VECTOR vector) {
    return Position{ vector.x, vector.y, vector.z };
}

static void addToRecordingFile(HandSensorData& data) {
    int msOffset = GetTickCount() - recordingTimeStart;
    TimedHandSensorData timedData{ msOffset, data };
    DWORD bytesWritten;
    if (!WriteFile(
        recordingFile,                // open file handle
        &data,      // start of data to write
        sizeof(TimedHandSensorData),  // number of bytes to write
        &bytesWritten, // number of bytes that were written
        NULL)) {
        printf("Error writing to file\n");
    }
}

/** Callback for when a frame of tracking data is available. */
static void OnFrame(const LEAP_TRACKING_EVENT *frame){
  if (frame->info.frame_id % 60 == 0)
    printf("Frame %lli with %i hands.\n", (long long int)frame->info.frame_id, frame->nHands);

  if (frame->info.frame_id % SENSOR_DATA_FACTOR != 0)
      return;

  HandSensorData handData;

  for(uint32_t h = 0; h < frame->nHands; h++){
    const LEAP_HAND* hand = &frame->pHands[h];

    const eLeapHandType type = hand->type;
    HandData* current = (type == eLeapHandType_Right ? &handData.right : &handData.left);
    current->position = convertLeapPosition(hand->palm.position);
    current->visible = true;

    for (int i = 0; i < 5; i++) {
        current->fingers[i].position = convertLeapPosition(hand->digits[i].distal.next_joint);
    }
    
    sendSensorData(coreSocket, handData);

    if (recordingFile != NULL) {
        addToRecordingFile(handData);
    }

    printf("    Hand id %i is a %s hand with position (%f, %f, %f).\n",
                hand->id,
                (hand->type == eLeapHandType_Left ? "left" : "right"),
                hand->palm.position.x,
                hand->palm.position.y,
                hand->palm.position.z);


  }
}

static void OnLogMessage(const eLeapLogSeverity severity, const int64_t timestamp,
                         const char* message) {
  const char* severity_str;
  switch(severity) {
    case eLeapLogSeverity_Critical:
      severity_str = "Critical";
      break;
    case eLeapLogSeverity_Warning:
      severity_str = "Warning";
      break;
    case eLeapLogSeverity_Information:
      severity_str = "Info";
      break;
    default:
      severity_str = "";
      break;
  }
  printf("[%s][%lli] %s\n", severity_str, (long long int)timestamp, message);
}

static void* allocate(uint32_t size, eLeapAllocatorType typeHint, void* state) {
  void* ptr = malloc(size);
  return ptr;
}

static void deallocate(void* ptr, void* state) {
  if (!ptr)
    return;
  free(ptr);
}

void OnPointMappingChange(const LEAP_POINT_MAPPING_CHANGE_EVENT *change){
  if (!connectionHandle)
    return;

  uint64_t size = 0;
  if (LeapGetPointMappingSize(*connectionHandle, &size) != eLeapRS_Success || !size)
    return;

  LEAP_POINT_MAPPING* pointMapping = (LEAP_POINT_MAPPING*)malloc((size_t)size);
  if (!pointMapping)
    return;

  if (LeapGetPointMapping(*connectionHandle, pointMapping, &size) == eLeapRS_Success &&
      pointMapping->nPoints > 0) {
    printf("Managing %u points as of frame %lld at %lld\n", pointMapping->nPoints, (long long int)pointMapping->frame_id, (long long int)pointMapping->timestamp);
  }
  free(pointMapping);
}

void OnHeadPose(const LEAP_HEAD_POSE_EVENT *event) {
  printf("Head pose:\n");
  printf("    Head position (%f, %f, %f).\n",
    event->head_position.x,
    event->head_position.y,
    event->head_position.z);
  printf("    Head orientation (%f, %f, %f, %f).\n",
    event->head_orientation.w,
    event->head_orientation.x,
    event->head_orientation.y,
    event->head_orientation.z);
  printf("    Head linear velocity (%f, %f, %f).\n",
    event->head_linear_velocity.x,
    event->head_linear_velocity.y,
    event->head_linear_velocity.z);
  printf("    Head angular velocity (%f, %f, %f).\n",
    event->head_angular_velocity.x,
    event->head_angular_velocity.y,
    event->head_angular_velocity.z);
}

int main(int argc, char** argv) {
    if (!(argc == 2 || argc == 3)) {
        fprintf(stderr, "[ERROR] Expected 1 argument (address of core instance).\n");
        exit(1);
    }

    char* core_address = argv[1];
    coreSocket = makeCoreSocket(core_address, PROXY_PORT);

    if (coreSocket == NULL) {
        exit(1);
    }

    printf("Connection with core service established.\n");

    if (argc == 3) {
        printf("In recording mode, opening file %s\n", argv[2]);
        char* fileToWrite = argv[2];
        recordingFile = CreateFile(fileToWrite, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (recordingFile == INVALID_HANDLE_VALUE) {
            printf("Failed to create/open file %s.\n", argv[2]);
        }

        DWORD currentTime = GetTickCount(); 
        recordingTimeStart = currentTime;
    }

    //Set callback function pointers
    ConnectionCallbacks.on_connection          = &OnConnect;
    ConnectionCallbacks.on_device_found        = &OnDevice;
    ConnectionCallbacks.on_frame               = &OnFrame;
    ConnectionCallbacks.on_point_mapping_change = &OnPointMappingChange;
    ConnectionCallbacks.on_log_message         = &OnLogMessage;
    ConnectionCallbacks.on_head_pose           = &OnHeadPose;

    connectionHandle = OpenConnection();
    {
    LEAP_ALLOCATOR allocator = { allocate, deallocate, NULL };
    LeapSetAllocator(*connectionHandle, &allocator);
    }
    LeapSetPolicyFlags(*connectionHandle, eLeapPolicyFlag_Images | eLeapPolicyFlag_MapPoints, 0);

    printf("Press Enter to exit program.\n");
    getchar();
  
    CloseConnection();
    DestroyConnection();
    closeCoreSocket(coreSocket);

    if (recordingFile) {
        CloseHandle(recordingFile);
    }

    return 0;
}
