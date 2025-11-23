#include "Player.h"
#include "GameState.h"
#include <iostream>
#include "collider.h"

Player::Player()
    : Movable(), state(PlayerState::IDLE),
    dashSpeed(150.0f), // update dash speed after movement normalization
    dashDuration(0), dashDurationMax(700),
    dashCooldown(5000), dashCooldownMark(0),
    canDash(true), targetEntity(nullptr),
    interactableNearby(nullptr), interactionRange(50.0f),
    directionIndex(2) {

    type = EntityType::PLAYER;
    id = 100;
    spriteWidth = 128.0f;
    spriteHeight = 128.0f;
    scale = 0.3f;
    maxSpeedX = 50.0f;
    maxSpeedY = 50.0f;
    solid = true;
    input.x = 0;
    input.y = 0;
    base_acceleration = 700.0f;
    base_deceleration = 5000.0f;
    setFriction(base_deceleration);
    maxSpeedX = 50.0f;
    maxSpeedY = 50.0f;

    collider = Collider(49.0f * scale, 45.0f * scale, 39.0f * scale, 39.0f * scale);
}

void Player::update(float deltaTime, GameState& gs) {
    // Update dash cooldown
    uint64_t nowTime = SDL_GetTicks();
    if (dashCooldownMark > 0) {
        dashDuration = nowTime - dashCooldownMark;
        if (dashDuration > dashCooldown) {
            dashCooldownMark = 0;
            dashDuration = 0;
            canDash = true;
        }
    }

    // State machine
    switch (state) {
    case PlayerState::IDLE: {
        if (currentAnimation != 1) {
            playAnimation(1);
            texture = gs.getResourceManager().getTexture("player_idle");
        }
        if (directionHorizontal != 0 || directionVertical != 0) {
            state = PlayerState::RUNNING;
        }
        //clampVelocity(deltaTime);
        //velocity += acceleration * deltaTime;
        applyMovement(deltaTime, maxSpeedX, maxSpeedY);
        break;
    }

    case PlayerState::RUNNING: {
        if (currentAnimation != 0) {
            playAnimation(0);
            texture = gs.getResourceManager().getTexture("player_run");
        }
        if (directionHorizontal == 0 && directionVertical == 0) {
            state = PlayerState::IDLE;
        }
        //clampVelocity(deltaTime);
        //velocity += acceleration * deltaTime;
        applyMovement(deltaTime, maxSpeedX, maxSpeedY);
        break;
    }

    case PlayerState::DASHING: {

            texture = gs.getResourceManager().getTexture("player_roll");


            if (currentAnimation != 2) {
                playAnimation(2);
                texture = gs.getResourceManager().getTexture("player_roll");
            }

            if (dashCooldownMark != 0) {
                if (dashDuration > dashDurationMax) {
                    state = PlayerState::IDLE;
                    canDash = false;
                }
                else {
                    float t = dashSpeed;
                    velocity = glm::vec2(directionHorizontal * dashSpeed, directionVertical * dashSpeed);
                    position += velocity * deltaTime;
                }
            }
            break;
        }
    }

    stepAnimation(deltaTime);
    if (acceleration.x != 0 || acceleration.y != 0) {
        Player::verticalSpriteIndex = getDirectionIndex();
    }
}


void Player::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    if (!texture) return;


    float srcX = currentAnimation != -1 ? animations[currentAnimation].currentFrame() * spriteWidth : 0.0f;

    float srcY = Player::verticalSpriteIndex * spriteHeight;

    SDL_FRect src = { 
        srcX,
        srcY,
        spriteWidth,
        spriteHeight };
    SDL_FRect dst = {
        position.x - viewport.x,
        position.y - viewport.y,
        spriteWidth * scale,
        spriteHeight * scale
    };

    SDL_RenderTexture(renderer, texture, &src, &dst);
}

void Player::handleCollision(Entity* other) {
    if (!other || !other->isSolid()) return;

    SDL_FRect myRect = getBoundingBox();
    SDL_FRect otherRect = other->getBoundingBox();
    SDL_FRect intersection;

    if (SDL_GetRectIntersectionFloat(&myRect, &otherRect, &intersection)) {
        if (intersection.w < intersection.h) {
            if (velocity.x > 0) {
                position.x -= intersection.w;
            }
            else if (velocity.x < 0) {
                position.x += intersection.w;
            }
            velocity.x = 0;
        }
        else {
            if (velocity.y > 0) {
                position.y -= intersection.h;
            }
            else if (velocity.y < 0) {
                position.y += intersection.h;
            }
            velocity.y = 0;
        }
    }
}

void Player::handleInput(const bool* keyState, GameState& gs) {

    current_acceleration = glm::vec2(0.0f, 0.0f);
    struct MoveDirectionSet {
        float X = 0.0;
        float Y = 0.0;
    };

    MoveDirectionSet movementDirectionSet{ 0, 0 };

    if (keyState[SDL_SCANCODE_A]) movementDirectionSet.X -= 1.0f;
    if (keyState[SDL_SCANCODE_D]) movementDirectionSet.X += 1.0f;
    if (keyState[SDL_SCANCODE_W]) movementDirectionSet.Y -= 1.0f;
    if (keyState[SDL_SCANCODE_S]) movementDirectionSet.Y += 1.0f;

    setDirection(movementDirectionSet.X, movementDirectionSet.Y);

    if (movementDirectionSet.X != 0 || movementDirectionSet.Y != 0) {

        glm::vec2 inputDir = glm::vec2(movementDirectionSet.X, movementDirectionSet.Y);

        inputDir = glm::normalize(inputDir);

        current_acceleration = inputDir * base_acceleration;

        input = inputDir;
    }
    else {
        // No keys pressed
        input = glm::vec2(0.0f, 0.0f);
    }

    acceleration = current_acceleration;


    // Dash
    if (keyState[SDL_SCANCODE_L] && canDash && (movementDirectionSet.X != 0 || movementDirectionSet.Y != 0)) {
        startDash();
    }

    // Attack
    if (keyState[SDL_SCANCODE_K]) {
        attack(gs);
    }
}

void Player::handleMovement(const bool* keyState, GameState &gs) {

}

void Player::startDash() {
    if (!canDash) return;
    state = PlayerState::DASHING;
    dashCooldownMark = SDL_GetTicks();
}

void Player::attack(GameState& gs) {
    // Will implement projectile creation later
    std::cout << "Player attacks!" << std::endl;
}