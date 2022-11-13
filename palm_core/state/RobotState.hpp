#pragma once

#include <vector>
#include "Obstacles.hpp"
#include "../lib/glm/glm.hpp"

enum PlannerType {
    LINEAR // Default
};

enum GestureType {
    DEFAULT
};

enum DeltaType {
    IDENTITY // Default
};

enum GestureAction {
	NONE,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

struct RobotState {
	glm::vec3 position;
};

struct SceneRobotState {
	std::vector<RobotState> robots;
};

struct ActualRobotState {
	std::vector<RobotState> robots;
};