#pragma once
#include "timer.h"

class Animation {
	Timer timer;
	int frameCount;
	int currentFrameIndex;   // cached frame


public:
	Animation() : timer(0), frameCount(0) {} // null constructor


	// parra constructor
	Animation(int frameCount, float lenght) : frameCount(frameCount), timer(lenght) {

	}

	float getLenght() const { return timer.getLenght(); }


	int currentFrame() const {
		return currentFrameIndex;
	}


	void step(float deltaTime) {
		timer.step(deltaTime);

		currentFrameIndex = static_cast<int>(timer.getTime() / timer.getLenght() * frameCount);
	}

};