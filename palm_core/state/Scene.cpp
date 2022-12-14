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
#define PLAN_THRESHOLD 2.5

ActualRobotState PalmScene::handleReceiveRobotState(const ActualRobotState& actualState){
    ActualRobotState returnState;
    
    actualRobotState = actualState;

    if(!hasInitialState){
        // Initial state has not been setup.
        state.robots = actualState.robots;
        hasInitialState = true;
        num_robots = state.robots.size();
        printf("[PalmScene::handleReeiveRobotState] Initialized with %d robots.\n", num_robots);
    }
    pthread_mutex_lock(&sceneMutex);
    SceneRobotState deltaState = Delta(this->state, actualState, handData);
    
    float deltaActualDiff = dot(deltaState, toActualRobotState(this->state)); 
    // Check if the new planned state is sufficiently far from our existing goal state
    if(deltaActualDiff < PLAN_THRESHOLD){
        // No new planning, use old trajectory to previous goal state. 
        // Check if the front of the trajectory queue is close to the current position
        // If so, pop it off of the queue.
    } else {
        printf("[PalmScene::handleReceiveRobotState] Planning to new position '%s' with diff %f\n", deltaState.toString().c_str(), deltaActualDiff);
        this->state = deltaState;
        // Need to plan to that new state
        std::vector<ActualRobotState> trajectory = Plan(actualState, deltaState);
        while(!trajectoryQueue.empty()){
            trajectoryQueue.pop();
        }
        for(size_t i = 0; i < trajectory.size(); i++){
            trajectoryQueue.push(trajectory[i]);
        }
        printf("Set %d trajectories\n", trajectoryQueue.size());
    }

    if(!trajectoryQueue.empty()){
        returnState = ActualRobotState{trajectoryQueue.front().robots};
        // If the current actual state is close enough to the trajectory state, pop it.
        float distance = dot(SceneRobotState{actualState.robots}, returnState);
        while(distance < TRAJECTORY_THRESHOLD && !trajectoryQueue.empty()){
            //printf("Target state close enough to goal state. Queue size: %d\n Target: %s\n Actual: %s, Return: %s, Distance: %f\n", 
            //    trajectoryQueue.size(), deltaState.toString().c_str(), actualState.toString().c_str(), returnState.toString().c_str(), distance);
            trajectoryQueue.pop();
            if(!trajectoryQueue.empty())
                returnState = ActualRobotState{trajectoryQueue.front().robots};

            distance = dot(SceneRobotState{actualState.robots}, returnState);
        }
    } 
    if(trajectoryQueue.empty()){
        returnState = actualState;
    }
    pthread_mutex_unlock(&sceneMutex);
    return returnState;
}

SceneRobotState PalmScene::Delta(const SceneRobotState& state, const ActualRobotState& actualState, HandDataQueue& handData){
    switch(configuration.deltaType){
        case DeltaType::IDENTITY:
            return Delta_Identity(state, actualState, handData);
        case DeltaType::GESTURE:
            return Delta_Gesture(state, actualState, handData);
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
        case PlannerType::POTENTIAL:
            return Plan_Potential(state, target);
        case PlannerType::POTENTIAL_GRADIENT:
            return Plan_Potential_Gradient(state, target);
        case PlannerType::RRT:
            return Plan_Linear(state, target);
    }
}