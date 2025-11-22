#include "Player.h"
#include "GameState.h"
#include <iostream>

Player::Player()
    : Movable(), state(PlayerState::IDLE),
    dashSpeedX(150.0f), dashSpeedY(105.0f),
    dashDuration(0), dashDurationMax(700),
    dashCooldown(5000), dashCooldownMark(0),
    canDash(true), targetEntity(nullptr),
    interactableNearby(nullptr), interactionRange(50.0f),
    baseAcceleration(700.0f) {

    type = EntityType::PLAYER;
    id = 100;
    spriteWidth = 128.0f;
    spriteHeight = 128.0f;
    scale = 0.3f;
    maxSpeedX = 50.0f;
    maxSpeedY = 35.0f;

    collider = Collider(49.0f, 45.0f, 39.0f, 39.0f);
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
        if (directionHorizontal != 0 || directionVertical != 0) {
            state = PlayerState::RUNNING;
        }
        else {
            applyFriction(deltaTime, 900.0f);
        }
        applyMovement(deltaTime);
        break;
    }

    case PlayerState::RUNNING: {
        if (directionHorizontal == 0 && directionVertical == 0) {
            state = PlayerState::IDLE;
        }
        applyMovement(deltaTime);
        break;
    }

    case PlayerState::DASHING: {
        if (dashCooldownMark != 0) {
            if (dashDuration > dashDurationMax) {
                state = PlayerState::RUNNING;
                canDash = false;
            }
            else {
                // Continue dash
                velocity = glm::vec2(
                    directionHorizontal * dashSpeedX,
                    directionVertical * dashSpeedY
                );
                position += velocity * deltaTime;
            }
        }
        else {
            dashCooldownMark = SDL_GetTicks();
        }
        break;
    }
    }

    // Update animation
    stepAnimation(deltaTime);
    Player::verticalSpriteIndex = getDirectionIndex();
}

void Player::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    if (!texture) return;

    float srcX = 0;
    if (currentAnimation >= 0 && currentAnimation < animations.size()) {
        srcX = animations[currentAnimation].currentFrame() * spriteWidth;
    }
    float srcY = Player::verticalSpriteIndex * spriteHeight;

    SDL_FRect src = { srcX, srcY, spriteWidth, spriteHeight };
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
    float inputX = 0;
    float inputY = 0;

    if (keyState[SDL_SCANCODE_A]) inputX -= 1;
    if (keyState[SDL_SCANCODE_D]) inputX += 1;
    if (keyState[SDL_SCANCODE_W]) inputY -= 1;
    if (keyState[SDL_SCANCODE_S]) inputY += 1;

    setDirection(inputX, inputY);

    if (state != PlayerState::DASHING) {
        acceleration = glm::vec2(inputX * baseAcceleration, inputY * baseAcceleration);
    }

    // Dash
    if (keyState[SDL_SCANCODE_L] && canDash && (inputX != 0 || inputY != 0)) {
        startDash();
    }

    // Attack
    if (keyState[SDL_SCANCODE_K]) {
        attack(gs);
    }
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