#pragma once
#include "RobotState.hpp"
#include <vector>
#include <queue>
#include "Utilities.hpp"
#include "../tracking/Tracking.hpp"
#include "Planning.hpp"
#include <optional>

struct PalmSceneConfiguration {
	std::vector<glm::vec3> robotStartingPositions;
	std::vector<Obstacle> obstacles;
	int robotCount = 1;
	DeltaType deltaType = DeltaType::IDENTITY;
	GestureType gestureType = GestureType::DEFAULT;
	PlannerType plannerType = PlannerType::LINEAR;
};

/**
 * The overarching class managing the entire state of PALM.
*/
class PalmScene {
private:
	pthread_mutex_t sceneMutex;

	SceneRobotState state;
	ActualRobotState actualRobotState;
	std::queue<ActualRobotState> trajectoryQueue;
	SceneObstacles obstacles;
	HandDataQueue handData;
	PalmSceneConfiguration configuration;
	
	SceneRobotState Delta(const SceneRobotState& state, const ActualRobotState& actualState, const HandDataQueue& handData);
	GestureAction Gesture(const HandDataQueue& handData);
	std::vector<ActualRobotState> Plan(const ActualRobotState& state, const SceneRobotState& target);
public:
	PalmScene(PalmSceneConfiguration config) {
		configuration = config;
		//actualRobotState = config.robotStartingPositions;
		obstacles = SceneObstacles{config.obstacles};
	};
	// Event Handling
	void handleSensorData(HandSensorData& sensorData);
	/**
	 * Returns the (local) nearby state that we want to map to (high refresh rate).
	*/
	ActualRobotState handleReceiveRobotState(const ActualRobotState& state);
	
};