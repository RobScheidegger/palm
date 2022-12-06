#include "Planning.hpp"
#include "Utilities.hpp"
#include <stdio.h>
#include <random>
#include <limits>
#define NORMALIZE_CONSTANT 50.0f

const static float INF = std::numeric_limits<float>::infinity();

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

#define ROBOT_RADIUS 0.3
float Potential_Goal(const ActualRobotState& state, const SceneRobotState& target){
    return 1.0f / dot(target, state);
}

float Potential_Robot(const RobotState& robotState1, const RobotState& robotState2){
    float d = glm::length(robotState1.position - robotState2.position);
    if(d < 2 * ROBOT_RADIUS){
        return INF;
    } else {
        return 1.0f / (d - 2 * ROBOT_RADIUS);
    }
}

float Potential_Robots(const ActualRobotState& state){
    float V = 0;
    // Compute for each pair of robots
    for(int i = 0; i < state.robots.size(); i++){
        for(int j = i + 1; j < state.robots.size(); j++){
            V += Potential_Robot(state.robots[i], state.robots[j]);
        }
    }
    return V;
}

#define MU_G 1000.0f
#define MU_R 1.0f
#define MU_O 1.0f
float Potential(const ActualRobotState& state, const SceneRobotState& target){
    return MU_R * Potential_Robots(state) - MU_G * Potential_Goal(state, target); 
}

#define TRAJECTORY_MAX 10
#define TRAJECTORY_DEVIATION 0.25
#define POTENTIAL_SAMPLES 1000
std::vector<ActualRobotState> Plan_Potential(const ActualRobotState& state, const SceneRobotState& target){
    std::vector<ActualRobotState> trajectories;
    ActualRobotState currentState = state;
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d{0,TRAJECTORY_DEVIATION};

    while(dot(target, currentState) > CLOSE_THRESHOLD && trajectories.size() < TRAJECTORY_MAX){
        // Gist: Explore outwards from the current state, sampling different potential functions at that point, and pick the best one
        ActualRobotState choice = currentState;
        float bestPotential = INF;
        for(int i = 0; i < POTENTIAL_SAMPLES; i++){
            // First: Make new vector from current with arbitrary 
            ActualRobotState sampleState{currentState.robots};
            for(int j = 0; j < sampleState.robots.size(); j++){
                // Add arbitary noise to the position of each robot
                sampleState.robots[j].position.x += d(gen); 
                sampleState.robots[j].position.y += d(gen); 
                sampleState.robots[j].position.z += d(gen); 
            }

            float potential = Potential(sampleState, target);
            if(potential < bestPotential){
                bestPotential = potential;
                choice = sampleState;
            }
        }
        trajectories.push_back(choice);
        currentState = choice;
        printf("Adding Potential Target: %s, potential: %f\n", choice.toString().c_str(), bestPotential);
    }

    return trajectories;
}