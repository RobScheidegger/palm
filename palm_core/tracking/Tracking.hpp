#pragma once

enum HandType {
	LEFT,
	RIGHT
};

struct Position {
	float x;
	float y;
	float z;
};

struct FingerData {
	Position position;
};

struct HandData {
	bool visible;
	Position position;
	FingerData fingers[5];
};

struct HandSensorData {
	HandData left;
	HandData right;

	HandSensorData() {
		left.visible = false;
		left.position = Position{ 0,0,0 };
		right.visible = false;
		right.position = Position{ 0,0,0 };

		for (int i = 0; i < 5; i++) {
			left.fingers[i].position = Position{ 0,0,0 };
		}
	}
};


