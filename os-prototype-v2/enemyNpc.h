#pragma once
#include "NPC.h"
#include "Timer.h"

enum class EnemyState {
    IDLE,
    PATROLLING,
    CHASING,
    ATTACKING,
    RETREATING,
    DEAD
};

enum class EnemyAIType {
    MELEE,
    RANGED,
    TANK,
    FAST
};

class EnemyNPC : public NPC {
private:
    EnemyState state;
    EnemyAIType aiType;

    float health;
    float maxHealth;
    float attackDamage;
    float attackRange;
    Timer attackCooldown;

    // AI behavior
    std::vector<glm::vec2> patrolPoints;
    int currentPatrolIndex;
    float chaseSpeed;
    float retreatThreshold; // Health % to retreat

public:
    EnemyNPC(EnemyAIType type);

    void update(float deltaTime, class GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override;
    void onPlayerInteract(class Player* player) override;

    // Combat
    void takeDamage(float damage);
    void attackTarget();
    bool isDead() const;

    // AI
    void updateAI(float deltaTime, class GameState& gs);
    void patrol(float deltaTime);
    void chaseTarget(float deltaTime);
    void attackSequence(float deltaTime);
    void retreat(float deltaTime);
};