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

// NEW: Enemy tier classification
enum class EnemyTier {
    TRASH_MOB,
    ELITE,
    BOSS
};

// NEW: Specific enemy types for damage tuning
enum class EnemyVariant {
    BEAST_MELEE,        // Fast, moderate damage
    HALBERD_FIGHTER,    // Medium speed, high damage
    BOSS_MELEE          // Slow, very high damage
};

// NEW: Enemy stats struct for different types
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
    static EnemyStats getStatsForVariant(EnemyVariant variant); // NEW: Variant-specific stats
};

class EnemyNPC : public NPC {
private:
    EnemyState state;
    EnemyAIType aiType;
    EnemyTier tier;
    EnemyVariant variant; // NEW: Specific enemy type

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

    // NEW: Texture management for animations
    std::string textureKey;
    SDL_Texture* idleTexture;
    SDL_Texture* walkTexture;
    SDL_Texture* attackTexture;

    // NEW: Animation state
    int currentAnimFrame;
    float animTimer;
    float animFrameDuration;
    int idleFrameCount;
    int walkFrameCount;
    int attackFrameCount;

    // NEW: Attack hitbox
    SDL_FRect attackHitbox;
    bool isAttacking;
    float attackAnimDuration;
    float attackAnimTimer;

    // NEW: Target highlight
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

    // NEW: Animation system
    void updateAnimation(float deltaTime);
    void playIdleAnimation();
    void playWalkAnimation();
    void playAttackAnimation();

    // NEW: Texture loading
    void loadTextures(class ResourceManager& rm);
    SDL_FRect getAttackHitbox() const;

    // NEW: Targeting visual
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
};