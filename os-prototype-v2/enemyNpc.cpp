#include "EnemyNpc.h"
#include "GameState.h"
#include "Player.h"
#include <iostream>

EnemyNPC::EnemyNPC(EnemyAIType type)
    : NPC(NPCType::HOSTILE), state(EnemyState::IDLE),
    aiType(type), health(100.0f), maxHealth(100.0f),
    attackDamage(10.0f), attackRange(50.0f), attackCooldown(1.5f),
    currentPatrolIndex(0), chaseSpeed(80.0f), retreatThreshold(20.0f) {

    this->type = EntityType::ENEMY;
    this->solid = true;
    this->spriteWidth = 32.0f;
    this->spriteHeight = 32.0f;
    this->friction = 500.0f; // Slidier than player
    this->maxSpeedX = 35.0f;
    this->maxSpeedY = 35.0f;


    // Initialize patrol points with at least current position to prevent crashes
    patrolPoints.push_back(glm::vec2(0, 0));
}

void EnemyNPC::update(float deltaTime, GameState& gs) {
    // 1. Check Death
    if (health <= 0) {
        state = EnemyState::DEAD;
        active = false;
        return;
    }

    // 2. Update AI Logic
    updateAI(deltaTime, gs);

    // 3. Apply Physics (from Movable)
    applyMovement(deltaTime, maxSpeedX, maxSpeedY);
}

void EnemyNPC::updateAI(float deltaTime, GameState& gs) {
    Player* player = gs.getPlayer();
    if (!player) return;

    float distToPlayer = glm::distance(position, player->getPosition());

    // Simple State Machine
    switch (state) {
    case EnemyState::IDLE:
        if (distToPlayer < detectionRange) {
            state = EnemyState::CHASING;
            setTarget(player);
        }
        break;

    case EnemyState::CHASING:
        if (distToPlayer > detectionRange * 1.5f) {
            state = EnemyState::IDLE;
            velocity = glm::vec2(0, 0);
        }
        else if (distToPlayer <= attackRange) {
            state = EnemyState::ATTACKING;
        }
        else {
            // Move towards player
            glm::vec2 dir = glm::normalize(player->getPosition() - position);
            acceleration = dir * chaseSpeed * 5.0f; // 5.0 arbitrary force multiplier
        }
        break;

    case EnemyState::ATTACKING:
        attackCooldown.step(deltaTime);
        if (attackCooldown.isTimeout()) {
            attackTarget();
            attackCooldown.reset();
        }
        if (distToPlayer > attackRange) {
            state = EnemyState::CHASING;
        }
        velocity = glm::vec2(0, 0); // Stop moving to attack
        break;

    case EnemyState::PATROLLING:
        patrol(deltaTime);
        break;
    }
}

void EnemyNPC::patrol(float deltaTime) {
    // CRASH FIX: Safety check
    if (patrolPoints.empty()) return;

    glm::vec2 targetPos = patrolPoints[currentPatrolIndex];
    if (glm::distance(position, targetPos) < 10.0f) {
        currentPatrolIndex = (currentPatrolIndex + 1) % patrolPoints.size();
    }
    else {
        glm::vec2 dir = glm::normalize(targetPos - position);
        acceleration = dir * (chaseSpeed * 0.5f);
    }
}

void EnemyNPC::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    // Simple red rectangle for enemy if texture missing
    if (!texture) {
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_FRect dst = { position.x - viewport.x, position.y - viewport.y, spriteWidth * scale, spriteHeight * scale };
        SDL_RenderFillRect(renderer, &dst);
    }
    else {
        // Normal render logic
        SDL_FRect dst = { position.x - viewport.x, position.y - viewport.y, spriteWidth * scale, spriteHeight * scale };
        SDL_RenderTexture(renderer, texture, nullptr, &dst);
    }
}

void EnemyNPC::handleCollision(Entity* other) {
    // Basic slide collision
}

void EnemyNPC::onPlayerInteract(Player* player) {
    // Enemies usually don't talk, they bite
}

void EnemyNPC::takeDamage(float damage) {
    health -= damage;
}

void EnemyNPC::attackTarget() {
    // Stub: Deal damage to target
    // std::cout << "Enemy Attacks!" << std::endl;
}

bool EnemyNPC::isDead() const {
    return health <= 0;
}
