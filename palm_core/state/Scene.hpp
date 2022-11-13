#pragma once
#include "RobotState.hpp"
#include <vector>
#include "Utilities.hpp"
#include "../tracking/Tracking.hpp"

struct PalmSceneConfiguration {
	std::vector<glm::vec3> robotStartingPositions;
	std::vector<Obstacle> obstacles;
	int robotCount;
};

/**
 * The overarching class managing the entire state  
*/
class PalmScene {
	pthread_mutex_t sceneMutex;

	SceneRobotState state;
	SceneObstacles obstacles;
	std::vector<glm::vec3> currentRobotPositions;
	std::optional<std::vector<glm::vec3>> trajectory;
	FixedQueue<HandSensorData, 50> handData;

	int robotCount;

	PalmScene(PalmSceneConfiguration config): robotCount{config.robotCount}){

	}


	SceneRobotState Delta(const SceneRobotState& state, const std::vector<HandSensorData>& handData);

	GestureAction Gesture(const std::vector<HandSensorData>& handData);

	std::vector<std::vector<glm::vec3>>& Plan(const SceneRobotState& state, const SceneRobotState& target);


};