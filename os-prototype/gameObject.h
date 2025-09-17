#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>
#include "animation.h"

enum class PlayerState {
	idle, running
};

struct PlayerData {

	PlayerState state;
	PlayerData() {
		state = PlayerState::idle;
	}
};

struct Collider {
	float right;
	float left;
	float top;
	float bottom;
};

struct LevelData {
	// todo - implementation pending
};
struct EnemyData {
	// todo - implementation pending
};
union ObjectData {
	PlayerData player; 
	LevelData level;
	EnemyData enemy;
};

enum class ObjectType
{
	player, level, enemy
};

struct GameObject {
	ObjectType type;
	ObjectData data;


	glm::vec2 position, velocity, acceleration;

	float directionVertical;   // -1 = up , 1 = down
	float directionHorizontal; // -1 = left, 1 = right
	float maxSpeedX;
	float maxSpeedY;
	float sprite_width;
	float sprite_height;
	float scale;

	bool solid;

	Collider collider;

	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;


	GameObject() : data{ .level = LevelData() } {
		type = ObjectType::level;

		directionVertical = -1;
		directionHorizontal = 1;
		maxSpeedX = 0;
		maxSpeedY = 0;
		sprite_width = 0.0f;
		sprite_height = 0.0f;
		scale = 0.0f;
		solid = false;

		collider = {
		.right = 0.0f,
		.left = 0.0f,
		.top = 0.0f,
		.bottom = 0.0f
		};

		position = velocity = acceleration = glm::vec2(0);
		currentAnimation = -1;
		texture = nullptr;
	}
};