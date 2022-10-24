#include "Tracking.hpp"
#include <stdio.h>
#include <stdlib.h>

static HandData* handPosition;

HandData* getHandPosition() {
	return handPosition;
}

void setHandPosition(HandData data) {
	*handPosition = data;
}

void initializeHandPosition() {
	handPosition = (HandData*)malloc(sizeof(HandData));
}