#include "RobotState.hpp"
#include <stdio.h>
#include <string>

float dot(const SceneRobotState& state, const ActualRobotState& actualPositions) {
    assert(state.robots.size() == actualPositions.robots.size());
    float f = 0;
    for(int i = 0; i < state.robots.size(); i++){
        f += glm::length(state.robots[i].position - actualPositions.robots[i].position); 
    }
    return f;
}

std::string RobotState::toString() const{
    return std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z);
}

std::string SceneRobotState::toString() const{
    std::string acc;
    for(size_t i = 0; i < robots.size(); i++)
    {
        acc += "R" + std::to_string(i) + ": " + robots[i].toString() + "\n";
    }
    return acc;
}

std::string ActualRobotState::toString() const{
    std::string acc;
    for(size_t i = 0; i < robots.size(); i++)
    {
        acc += "R" + std::to_string(i) + ": " + robots[i].toString() + "\n";
    }
    return acc;
}

ActualRobotState toActualRobotState(const SceneRobotState& state){
    return ActualRobotState{state.robots};
}
SceneRobotState toSceneRobotState(const ActualRobotState& state){
    return SceneRobotState{state.robots};
}

ActualRobotState difference(const SceneRobotState& state1, const ActualRobotState& state2){
    assert(state1.robots.size() == state2.robots.size());
    std::vector<RobotState> robots;
    for(int i = 0; i < state1.robots.size(); i++){
        robots.push_back(RobotState{state1.robots[i].position - state2.robots[i].position});
    }
    return ActualRobotState{robots};
}

ActualRobotState scale(const ActualRobotState& state, const float f){
    std::vector<RobotState> robots;
    for(int i = 0; i < state.robots.size(); i++){
        robots.push_back(RobotState{state.robots[i].position * f});
    }
    return ActualRobotState{robots};
}

ActualRobotState add(const ActualRobotState& state1, const ActualRobotState& state2){
    assert(state1.robots.size() == state2.robots.size());
    std::vector<RobotState> robots;
    for(int i = 0; i < state1.robots.size(); i++){
        robots.push_back(RobotState{state1.robots[i].position + state2.robots[i].position});
    }
    return ActualRobotState{robots};
}