#pragma once

enum Hand {
	LEFT,
	RIGHT
};

struct Position {
	float x;
	float y;
	float z;
};

struct HandData {
	Hand hand;
	Position position;
};

HandData* getHandPosition();


