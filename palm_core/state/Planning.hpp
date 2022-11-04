#pragma once

#include "RobotState.hpp"
#include <vector>
#include "../tracking/Tracking.hpp"

SceneState Delta(const SceneState& state, const std::vector<HandSensorData>& handData);

GestureAction Gesture(const std::vector<HandSensorData>& handData);

std::vector<SceneState>& Plan(const SceneState& state, const SceneState& target);