#pragma once
#include "Movable.h"
#include "Timer.h"

enum class PlayerState {
    IDLE,
    RUNNING,
    DASHING
};

class Player : public Movable {
private:
    PlayerState state;

    // Dash system
    float dashSpeed;
    uint64_t dashDuration;
    float dashDurationMax;
    float dashCooldown;
    uint64_t dashCooldownMark;
    bool canDash;
    int directionIndex;

    // Combat
    Entity* targetEntity;

    // Interaction
    Entity* interactableNearby;
    float interactionRange;



public:
    Player();

    void update(float deltaTime, GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override;

    void handleInput(const bool* keyState, GameState& gs);
    void handleMovement(const bool* keyState, GameState& gs);
    void startDash();
    void attack(GameState& gs);

    PlayerState getState() const { return state; }
    void setState(PlayerState newState) { state = newState; }

    void setTarget(Entity* target) { targetEntity = target; }
    Entity* getTarget() const { return targetEntity; }
};