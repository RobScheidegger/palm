#pragma once
#include "RobotState.hpp"
#include "../tracking/Tracking.hpp"
#include "Utilities.hpp"

SceneRobotState Delta_Identity(const SceneRobotState& state, const ActualRobotState& actualState, const HandDataQueue& handData);

GestureAction Gesture_Default(const HandDataQueue& handData);

std::vector<ActualRobotState> Plan_Linear(const ActualRobotState& state, const SceneRobotState& target);
std::vector<ActualRobotState> Plan_Potential(const ActualRobotState& state, const SceneRobotState& target);
std::vector<ActualRobotState> Plan_Potential_Gradient(const ActualRobotState& state, const SceneRobotState& target);