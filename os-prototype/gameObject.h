#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>
#include "animation.h"

enum class ObjectType
{
	player, level, enemy
};

struct GameObject {
	ObjectType type;

	glm::vec2 position, velocity, acceleration;
	float directionVertical;   // -1 = up , 1 = down
	float directionHorizontal; // -1 = left, 1 = right
	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;


	GameObject() {
		type = ObjectType::level;
		directionVertical = -1;
		directionHorizontal = 1;
		position = velocity = acceleration = glm::vec2(0);
		currentAnimation = -1;
		texture = nullptr;
	}
};