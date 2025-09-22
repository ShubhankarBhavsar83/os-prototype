#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>
#include "animation.h"

enum class PlayerState {
	idle, running, dashing
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
struct FurnitureData {

};
union ObjectData {
	PlayerData player; 
	LevelData level;
	EnemyData enemy;
	FurnitureData furniture;

};

enum class ObjectType
{
	player, level, enemy, furniture
};

struct GameObject {
	ObjectType type;
	ObjectData data;

	int id;
	size_t currentLayer;

	glm::vec2 position, velocity, acceleration;

	float directionVertical;   // -1 = up , 1 = down
	float directionHorizontal; // -1 = left, 1 = right
	float maxSpeedX;
	float maxSpeedY;

	float dashAccel;
	float baseSpeed;
	float dashingSpeedX;
	float dashingSpeedY;
	long dashDuration;
	long dashDurationMax;
	float dashCooldown;
	long dashCooldownMark;
	bool dashOnCd;

	float sprite_width;
	float sprite_height;
	float scale;
	short verticalSpriteIndex; // up - 0, top right - 1, right - 2, bottom right - 3 , down - 4, bottom left - 5 , left - 6 , top left - 7

	bool solid;

	Collider collider;

	std::vector<Animation> animations;
	int currentAnimation;
	SDL_Texture* texture;


	GameObject() : data{ .level = LevelData() } {
		type = ObjectType::level;

		id = 1;
		currentLayer = 0;

		directionVertical = -1;
		directionHorizontal = 1;
		maxSpeedX = 0;
		maxSpeedY = 0;
		baseSpeed = 50;

		dashOnCd = false;
		dashAccel = 700;
		dashingSpeedX = 100;
		dashingSpeedY = 70;
		dashDuration = 0;
		dashDurationMax = 700;
		dashCooldown = 5000;
		dashCooldownMark = 0.0f;


		sprite_width = 0.0f;
		sprite_height = 0.0f;
		scale = 0.0f;
		solid = false;
		verticalSpriteIndex = 0;

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