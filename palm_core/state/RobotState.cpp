#include "RobotState.hpp"

float dot(const SceneRobotState& state, const ActualRobotState& actualPositions) {
    assert(state.robots.size() == actualPositions.robots.size());
    float f = 0;
    for(int i = 0; i < state.robots.size(); i++){
        f += glm::dot(state.robots[i].position, actualPositions.robots[i].position); 
    }
    return f;
}