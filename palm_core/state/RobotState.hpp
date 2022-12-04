#pragma once

#include <vector>
#include "Obstacles.hpp"
#include "../lib/glm/glm.hpp"
#include <string>

enum PlannerType {
    LINEAR, // Default
	POTENTIAL,
	RRT
};

enum GestureType {
    DEFAULT
};

enum DeltaType {
    IDENTITY // Default
};

enum GestureAction {
	GESTURE_NONE,
	GESTURE_LEFT,
	GESTURE_RIGHT,
	GESTURE_UP,
	GESTURE_DOWN
};

struct RobotState {
	glm::vec3 position{0,0,0};
	std::string toString();
};

struct SceneRobotState {
	std::vector<RobotState> robots = std::vector<RobotState>{};
	std::string toString();
};

struct ActualRobotState {
	std::vector<RobotState> robots = std::vector<RobotState>{};
	std::string toString();
};

float dot(const SceneRobotState& state, const ActualRobotState& actualPositions);

ActualRobotState toActualRobotState(const SceneRobotState& state);
SceneRobotState toSceneRobotState(const ActualRobotState& state);

ActualRobotState difference(const SceneRobotState& state1, const ActualRobotState& state2);
ActualRobotState scale(const ActualRobotState& state, const float f);
ActualRobotState add(const ActualRobotState& state1, const ActualRobotState& state2);