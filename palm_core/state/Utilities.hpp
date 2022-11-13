#pragma once
#include <queue>
#include "RobotState.hpp"
#include "../tracking/Tracking.hpp"

template <typename T, int MaxLen, typename Container=std::deque<T>>
class FixedQueue : public std::queue<T, Container> {
public:
    void push(const T& value) {
        if (this->size() == MaxLen) {
           this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }
};

typedef FixedQueue<HandSensorData, 50> HandDataQueue;

float dot(const ActualRobotState& state, const ActualRobotState& actualPositions){
    return dot(SceneRobotState{state.robots}, actualPositions);
}

float dot(const SceneRobotState& state, const ActualRobotState& actualPositions) {
    float f = 0;
    for(int i = 0; i < state.robots.size(); i++){
        f += glm::dot(state.robots[i].position, actualPositions.robots[i].position); 
    }
    return f;
}