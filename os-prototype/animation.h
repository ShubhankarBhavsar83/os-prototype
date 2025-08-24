#pragma once
#include "timer.h"

class Animation {
	Timer timer;
	int frameCount;		

public:
	Animation() : timer(0) , frameCount(0) {}
	Animation(int frameCount, float lenght) : timer(lenght), frameCount(frameCount) {
	
	}

	float getLenght() const { return timer.getLenght(); }
	int currentFrame() const {
		return static_cast<int>(timer.getTime() / timer.getLenght() * frameCount);
	}

	void step(float deltaTime) {
		timer.step(deltaTime);
	}

};