#pragma once


class Timer {
	float lenght, time; 
	bool timeout;
public :
	Timer(float lenght) : lenght(lenght), time(0), timeout(false) {}

	void step(float deltaTime) {
		time += deltaTime;
		if (time > lenght) {
			time -= lenght;
			timeout = true;
		}
	}

	bool isTimeout() const {
		return timeout;
	}

	float getTime() const {
		return time;
	}

	float getLenght() const {
		return lenght;
	}

	void reset() {
		time = 0;
	}
};