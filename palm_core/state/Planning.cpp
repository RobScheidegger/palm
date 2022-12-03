#include "Planning.hpp"
#include "Utilities.hpp"
#include <stdio.h>

#define NORMALIZE_CONSTANT 50.0f

SceneRobotState Delta_Identity(const SceneRobotState& state, const ActualRobotState& actualState, const HandDataQueue& handData){
    SceneRobotState newRobotState;
    // Take the last one and set the ideal positions acordingly
    if(handData.empty()){
        return state;
    }
    const HandSensorData firstData = handData.back();
    
    if(!firstData.right.visible){
        return state;
    }
        
    int numRobots = state.robots.size();
    for(int i = 0; i < numRobots && i < 5; i++){
        const FingerData& finger = firstData.right.fingers[i];

        RobotState robot{glm::vec3(finger.position.z, finger.position.x, finger.position.y) / NORMALIZE_CONSTANT};
        newRobotState.robots.push_back(robot);
    }
    if(numRobots > 5){
        int n = numRobots - 5;
        for(int i = 0; i < n && i < 5; i++){
            const FingerData& finger = firstData.left.fingers[i];

            RobotState robot{glm::vec3(finger.position.z, finger.position.x, finger.position.y) / NORMALIZE_CONSTANT};
            newRobotState.robots.push_back(robot);
        }
    }

    return newRobotState;
}

GestureAction Gesture_Default(const HandDataQueue& handData){
    return GestureAction::GESTURE_NONE;
}

#define CLOSE_THRESHOLD 0.1f
#define INTERPOLATION_DELTA 2.5f
std::vector<ActualRobotState> Plan_Linear(const ActualRobotState& state, const SceneRobotState& target){
    std::vector<ActualRobotState> trajectories;

    // Otherwise, lineary interpolate between the current and target states
    float distance = dot(target, state);
    int interpolationFactor = distance / INTERPOLATION_DELTA;

    const ActualRobotState diff = scale(difference(target, state), 1.0f / interpolationFactor);
    for(int i = 0; i < interpolationFactor; i++){
        trajectories.push_back(add(state, scale(diff, (float)i)));
    }
    trajectories.push_back(ActualRobotState{target.robots});

    return trajectories;
}