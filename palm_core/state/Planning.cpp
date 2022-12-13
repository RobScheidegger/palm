#include "Planning.hpp"
#include "Utilities.hpp"
#include <stdio.h>
#include <random>
#include <limits>
#include <map>
#include <tuple>
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

#define ROBOT_RADIUS 0.1
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
#define MU_R 0//1.0f
#define MU_O 1.0f
float Potential(const ActualRobotState& state, const SceneRobotState& target){
    return MU_R * Potential_Robots(state) - MU_G * Potential_Goal(state, target); 
}

#define TRAJECTORY_MAX 20
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

ActualRobotState Potential_Goal_Gradient(const ActualRobotState& state, const SceneRobotState& target){
    float V_G = Potential_Goal(state, target);
    return scale(difference(target, state), -V_G * V_G * 2);
}


glm::vec3 Potential_Robot_Gradient(const RobotState& robotState1, const RobotState& robotState2){
    float V_R = Potential_Robot(robotState1, robotState2);

    float d = glm::length(robotState1.position - robotState2.position);
    return -1.0f * V_R * V_R / d  * (robotState1.position - robotState2.position);
}

ActualRobotState Potential_Robots_Gradient(const ActualRobotState& state){
    ActualRobotState V{state.robots};
    // Compute for each pair of robots
    for(int i = 0; i < state.robots.size(); i++){
        glm::vec3 grad{0,0,0};
        for(int j = 0; j < state.robots.size(); j++){
            if(i == j)
                continue;
            grad += Potential_Robot_Gradient(state.robots[i], state.robots[j]);
        }
        V.robots[i].position = grad;
    }
    return V;
}


#define ALPHA 1
#define MU_G_GRADIENT 3
#define MU_R_GRADIENT 0.1
ActualRobotState Potential_Gradient(const ActualRobotState& state, const SceneRobotState& target){
    return scale(Potential_Goal_Gradient(state, target), -MU_G_GRADIENT * ALPHA);
    //return add(scale(Potential_Goal_Gradient(state, target), -MU_G_GRADIENT * ALPHA), 
    //           scale(Potential_Robots_Gradient(state), MU_R_GRADIENT * ALPHA));
}

#define TRAJECTORY_MAX_POTENTIAL_GRADIENT 500
std::vector<ActualRobotState> Plan_Potential_Gradient(const ActualRobotState& state, const SceneRobotState& target){
    std::vector<ActualRobotState> trajectories;
    ActualRobotState currentState = state;
    int i = 0;
    printf("Gradient state: %s, target: %s\n", state.toString().c_str(), target.toString().c_str());

    // Use the same potential function as the sampling method, but we move along a gradient to the next point (gradient descent)
    while((trajectories.size() < TRAJECTORY_MAX_POTENTIAL_GRADIENT) && dot(target, currentState) >= 0.5){
        i++;
        currentState = add(currentState, Potential_Gradient(currentState, target));
        if(i % 25 == 0){
            trajectories.push_back(currentState);
            //printf("Adding Potential Target: %s, potential: %f\n", currentState.toString().c_str(), Potential(currentState, target));
        }
    }

    return trajectories;
}

std::vector<SceneRobotState> Random_State() {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::uniform_real_distribution<> d{0,1};

    std::vector<SceneRobotState> p;
    for(int i = 0; i < 10; i++){
        SceneRobotState state;
        for(int j = 0; j < 3; j++){
            RobotState robotState;
            robotState.position = glm::vec3{d(gen), d(gen), d(gen)};
            state.robots.push_back(robotState);
        }
        p.push_back(state);
    }
    return p;
}


#define GOAL_PROBABILITY 0.1
#define RAD 10
std::vector<ActualRobotState> RRT_Star(const ActualRobotState& state, const SceneRobotState& target){
    std::vector<ActualRobotState> trajectories;
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::uniform_real_distribution<> d{0,1};

    std::vector<ActualRobotState> x_rand;
    std::vector<ActualRobotState> x_near;
    std::vector<ActualRobotState> x_new;

    std::map<std::vector<ActualRobotState>, float> cost;

/*
    for(int idx = 0; idx < 10; idx++){

    // Set random positions to x_rand
    for(int i = 0; i < 10; i++){
        ActualRobotState state;
        for(int j = 0; j < ; j++){
            RobotState robotState;
            robotState.position = glm::vec3{d(gen), d(gen), d(gen)};
            state.robots.push_back(robotState);
        }
        x_rand.push_back(state);
    }

    // Set x_near to the nearest point in trajectories from x_rand
    ActualRobotState x_near;
    int smallest_distance = INF;
    for(int j = 0; j < trajectories.size(); j++){
        ActualRobotState vertex = trajectories[j];
        // find distance between vertex and x_rand
        float distance = dot(vertex, x_rand);
        if (distance < smallest_distance){
            smallest_distance = distance;
            x_near = vertex;
        }
    }

    cost.insert({ x_rand, smallest_distance });

    for(int j = 0; j < trajectories.size(); j++){
        std::vector<ActualRobotState> neighbors; 
        ActualRobotState vertex = trajectories[j];
        // find distance between vertex and x_rand
        int distance = distance_func(vertex, x_rand);
        if (distance < RAD)
            neighbors.push_back(vertex);
        if (distance < smallest_distance){
            smallest_distance = distance;
            x_near = vertex;
        }
    }

    for(int i = 0; i < neighbors.size(); i++){
        std::vector<ActualRobotState> neighbor = neighbors[i];
        float cost_new = cost.find(x_rand);
        float cost_neighbor = cost.find(neighbor);
        if (cost_new->second + dot(x_rand, neighbor) < cost_neighbor->second){
            cost.find(neighbor)->second = cost_new + dot(x_rand, neighbor);
            trajectories.push_back({x_rand, neighbor});
        }
    }

    trajectories.push_back({x_rand, x_near});

    }
*/
    return trajectories;  

}

struct RegressionResult{
    float b;
    float m;
    float r;
};

RegressionResult temporalRegression(std::vector<float> values){
    float n = values.size();
    float sx = n * (n - 1) / 2;

    float sy = 0;
    float sxx = 0;
    float sxy = 0;
    for(int i = 0; i < n; i++){
        sy += values[i];
        sxx += std::pow(float(i), 2);
        sxy += values[i] * (float)i;
    }
    float b = (sy * sxx - sx * sxy) / (n * sxx - sx * sx);
    float m = (n * sxy - sx * sy) / (n * sxx - sx * sx);
    float sres = 0;
    float stot = 0;
    float ymean = sy / n;
    for(int i = 0; i < n; i++){
        sres += std::pow(values[i] - (b + m * (float)i), 2);
        stot += std::pow(values[i] - ymean, 2);
    }
    float r = 1.0f - sres / stot;
    return {b, m, r};
}

#define R_THRESHOLD 0.85
#define DISTANCE_THRESHOLD 1
#define PAST_QUEUE_WINDOW 10
glm::vec3 Gesture_Linear(HandDataQueue& handData){
    // Perform regression on each of the points of handData to get the x values;
    std::vector<float> xValues;
    std::vector<float> yValues;
    std::vector<float> zValues;
    HandDataQueue queue = handData;

    while(!queue.empty() && xValues.size() <= PAST_QUEUE_WINDOW){
        HandSensorData& hand = queue.front();
        queue.pop();
        xValues.push_back(hand.right.position.x);
        yValues.push_back(hand.right.position.y);
        zValues.push_back(hand.right.position.z);
    }

    int n = xValues.size();
    if(n < PAST_QUEUE_WINDOW)
        return glm::vec3{0,0,0};
    RegressionResult rx = temporalRegression(xValues);
    RegressionResult ry = temporalRegression(yValues);
    RegressionResult rz = temporalRegression(zValues);

    float ravg = (rx.r + ry.r + rz.r) / 3;
    float magnitude = glm::length(glm::vec3{rx.m, ry.m, rz.m});
    //printf("Linear regression with confidence: {%f, %f, %f}, avg: %f, mag: %f\n", rx.r, ry.r, rz.r, ravg, magnitude);
    if(ravg > R_THRESHOLD && magnitude >= DISTANCE_THRESHOLD){
        for(int i = 0; i < PAST_QUEUE_WINDOW && !handData.empty(); i++){
            handData.pop();
        }
        return glm::normalize(glm::vec3{rz.m, rx.m, ry.m});
    }
    return glm::vec3{0,0,0};
}

#define GESTURE_SCALE 1.0f
SceneRobotState Delta_Gesture(const SceneRobotState& state, const ActualRobotState& actualState, HandDataQueue& handData){
    float d = dot(state, actualState);
    // If far away, avoid since we assume some movement is going on
    if(d > CLOSE_THRESHOLD)
        return state;

    SceneRobotState newState = state;
    glm::vec3 gesture = Gesture_Linear(handData);
    for(int i = 0; i < state.robots.size(); i++){
        newState.robots[i].position += GESTURE_SCALE * gesture;
    }
    //if(glm::length(gesture) != 0)
    //    printf("Old: %s, New: %s\n", state.toString().c_str(), newState.toString().c_str());
    return newState;
}