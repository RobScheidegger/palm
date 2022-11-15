#include "Scene.hpp"
#include <pthread.h>
#include "Planning.hpp"
#include <stdio.h>

void PalmScene::handleSensorData(HandSensorData& data){
    pthread_mutex_lock(&sceneMutex);
    handData.push(data);
    pthread_mutex_unlock(&sceneMutex);
}

#define TRAJECTORY_THRESHOLD 0.1
#define PLAN_THRESHOLD 0.5

ActualRobotState PalmScene::handleReceiveRobotState(const ActualRobotState& actualState){
    ActualRobotState returnState;
    pthread_mutex_lock(&sceneMutex);
    actualRobotState = actualState;
    SceneRobotState deltaState = Delta(this->state, actualState, handData);
    if(dot(deltaState, actualState) < 0.5){
        // No new planning, use old trajectory. 
        // Check if the front of the trajectory queue is close to the current position
        // If so, pop it off of the queue.
    } else {
        // Need to plan to that new state
        std::vector<ActualRobotState> trajectory = Plan(actualState, deltaState);
        while(!trajectoryQueue.empty()){
            trajectoryQueue.pop();
        }
        for(int i = 0; i < trajectory.size(); i++){
            trajectoryQueue.push(trajectory[i]);
        }
    }
    if(!trajectoryQueue.empty() && dot(SceneRobotState{trajectoryQueue.front().robots}, actualRobotState) < TRAJECTORY_THRESHOLD){
        returnState = trajectoryQueue.front();
        trajectoryQueue.pop();
    } else {
        returnState = actualState;
    }
    pthread_mutex_unlock(&sceneMutex);
    return returnState;
}

SceneRobotState PalmScene::Delta(const SceneRobotState& state, const ActualRobotState& actualState, const HandDataQueue& handData){
    switch(configuration.deltaType){
        case DeltaType::IDENTITY:
            return Delta_Identity(state, actualState, handData);
    }
}

GestureAction PalmScene::Gesture(const HandDataQueue& handData){
    switch(configuration.gestureType){
        case GestureType::DEFAULT:
            return Gesture_Default(handData);
    }
}

std::vector<ActualRobotState> PalmScene::Plan(const ActualRobotState& state, const SceneRobotState& target){
    switch(configuration.plannerType){
        case PlannerType::LINEAR:
            return Plan_Linear(state, target);
    }
}