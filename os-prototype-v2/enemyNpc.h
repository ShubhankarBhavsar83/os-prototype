#pragma once
#include "NPC.h"
#include "Timer.h"

enum class EnemyState {
    IDLE,
    PATROLLING,
    CHASING,
    ATTACKING,
    RETREATING,
    DYING,      // NEW: Death animation state
    DEAD
};

enum class EnemyAIType {
    MELEE,
    RANGED,
    TANK,
    FAST
};

enum class EnemyTier {
    TRASH_MOB,
    ELITE,
    BOSS
};

enum class EnemyVariant {
    BEAST_MELEE,
    HALBERD_FIGHTER,
    BOSS_MELEE
};

struct EnemyStats {
    float maxHealth;
    float attackDamage;
    float attackRange;
    float attackCooldown;
    float detectionRange;
    float aggroRange;
    float moveSpeed;
    float scale;

    static EnemyStats getStatsForType(EnemyAIType aiType, EnemyTier tier);
    static EnemyStats getStatsForVariant(EnemyVariant variant);
};

class EnemyNPC : public NPC {
private:
    EnemyState state;
    EnemyAIType aiType;
    EnemyTier tier;
    EnemyVariant variant;

    // Combat stats
    float health;
    float maxHealth;
    float attackDamage;
    float attackRange;
    Timer attackCooldown;

    // AI behavior
    std::vector<glm::vec2> patrolPoints;
    int currentPatrolIndex;
    float chaseSpeed;
    float retreatThreshold;

    // Texture management for animations
    std::string textureKey;
    SDL_Texture* idleTexture;
    SDL_Texture* walkTexture;
    SDL_Texture* attackTexture;
    SDL_Texture* deathTexture;  // NEW: Death animation texture

    // Animation state
    int currentAnimFrame;
    float animTimer;
    float animFrameDuration;
    int idleFrameCount;
    int walkFrameCount;
    int attackFrameCount;
    int deathFrameCount;        // NEW: Death frame count

    // NEW: Death animation timing
    float deathAnimTimer;
    float deathAnimDuration;
    bool deathAnimComplete;

    // Attack hitbox
    SDL_FRect attackHitbox;
    bool isAttacking;
    float attackAnimDuration;
    float attackAnimTimer;

    // Target highlight
    bool isTargeted;
    float targetPulseTimer;

public:
    EnemyNPC(EnemyAIType type, EnemyTier tier = EnemyTier::TRASH_MOB,
        EnemyVariant variant = EnemyVariant::BEAST_MELEE);

    void update(float deltaTime, class GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override;
    void onPlayerInteract(class Player* player) override;

    // Combat
    void takeDamage(float damage);
    void attackTarget();
    bool isDead() const;
    float getHealthPercent() const { return health / maxHealth; }

    // AI
    void updateAI(float deltaTime, class GameState& gs);
    void patrol(float deltaTime);
    void chaseTarget(float deltaTime);
    void performAttack(float deltaTime);
    void retreat(float deltaTime);

    // Animation system
    void updateAnimation(float deltaTime);
    void playIdleAnimation();
    void playWalkAnimation();
    void playAttackAnimation();
    void playDeathAnimation();      // NEW: Death animation

    // Texture loading
    void loadTextures(class ResourceManager& rm);
    SDL_FRect getAttackHitbox() const;

    // Targeting visual
    void setTargeted(bool targeted) { isTargeted = targeted; }
    bool getTargeted() const { return isTargeted; }
    void renderTargetHighlight(SDL_Renderer* renderer, const SDL_FRect& viewport);

    // Getters
    EnemyTier getTier() const { return tier; }
    EnemyAIType getAIType() const { return aiType; }
    EnemyVariant getVariant() const { return variant; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getAttackDamage() const { return attackDamage; }
    bool isCurrentlyAttacking() const { return isAttacking; }
    EnemyState getState() const { return state; }  // NEW: State getter
};