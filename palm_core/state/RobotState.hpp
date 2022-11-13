#pragma once

#include <vector>
#include "Obstacles.hpp"
#include "../lib/glm/glm.hpp"

enum GestureAction {
	LEFT,
	RIGHT,
	UP,
	DOWN
};

struct RobotState {
	glm::vec3 position;
	glm::vec3 velocity;
};

class SceneRobotState {
	std::vector<RobotState> robots;
};