#pragma once
#include "Movable.h"
#include "Timer.h"
#include <vector>

enum class PlayerState {
    IDLE,
    RUNNING,
    DASHING,
    ATTACKING_MELEE,
    ATTACKING_RANGED
};

enum class TargetingMode {
    NEAREST,
    LOWEST_HP,
    HIGHEST_HP
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

    // NEW: Combat system
    Entity* targetEntity;
    std::vector<Entity*> enemiesInRange;
    int currentTargetIndex;
    float targetingRange;
    TargetingMode targetingMode;

    // NEW: Melee attack
    Timer meleeCooldown;
    float meleeRange;
    float meleeDamage;
    SDL_FRect meleeHitbox;
    bool isMeleeAttacking;
    float meleeAnimDuration;
    float meleeAnimTimer;

    // NEW: Ranged attack (fireball)
    Timer fireballCooldown;
    float fireballDamage;
    bool canCastFireball;

    // Interaction
    Entity* interactableNearby;
    float interactionRange;

    // NEW: HP system
    float health;
    float maxHealth;

public:
    Player();

    void update(float deltaTime, GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override;
    void handleInput(const bool* keyState, GameState& gs);

    // Movement
    void startDash();

    // NEW: Combat methods
    void meleeAttack(GameState& gs);
    void rangedAttack(GameState& gs); // Fireball
    void updateAttackHitbox();
    SDL_FRect getMeleeHitbox() const { return meleeHitbox; }
    bool isCurrentlyAttacking() const { return isMeleeAttacking; }

    // NEW: Targeting system
    void cycleTarget(GameState& gs); // TAB key
    void updateTargeting(GameState& gs);
    void findEnemiesInRange(GameState& gs);
    Entity* findNearestEnemy(GameState& gs);
    void clearTarget();

    // NEW: Interaction (polymorphic)
    void interact(GameState& gs);
    void checkForInteractables(GameState& gs);

    // NEW: HP system
    void takeDamage(float damage);
    void heal(float amount);
    float getHealthPercent() const { return health / maxHealth; }
    bool isDead() const { return health <= 0; }

    // Getters/Setters
    PlayerState getState() const { return state; }
    void setState(PlayerState newState) { state = newState; }
    Entity* getTarget() const { return targetEntity; }
    Entity* getInteractable() const { return interactableNearby; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
};
