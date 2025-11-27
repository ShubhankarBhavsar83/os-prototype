#include "EnemyNpc.h"
#include "GameState.h"
#include "Player.h"
#include <iostream>

// ============================================================================
// DAMAGE TUNING SECTION - Balanced for player combat
// ============================================================================

EnemyStats EnemyStats::getStatsForVariant(EnemyVariant variant) {
    EnemyStats stats;

    switch (variant) {
    case EnemyVariant::BEAST_MELEE:
        // BALANCED: 2 melee hits (50 dmg each) OR 3 fireballs (33.33 dmg each) to kill
        stats.maxHealth = 5000;
        stats.attackDamage = 20.0f;     // Player takes 5 hits to die (100 HP / 20 dmg)
        stats.attackRange = 35.0f;
        stats.attackCooldown = 1.0f;
        stats.detectionRange = 280.0f;
        stats.aggroRange = 200.0f;
        stats.moveSpeed = 65.0f;
        stats.scale = 0.35f;
        //texture = gs.getResourceManager().getTexture("player_ranged");
        break;

    case EnemyVariant::HALBERD_FIGHTER:
        // BALANCED: Same damage rules apply
        stats.maxHealth = 10000.0f;
        stats.attackDamage = 20.0f;
        stats.attackRange = 55.0f;
        stats.attackCooldown = 1.5f;
        stats.detectionRange = 250.0f;
        stats.aggroRange = 180.0f;
        stats.moveSpeed = 45.0f;
        stats.scale = 0.4f;
        break;

    case EnemyVariant::BOSS_MELEE:
        // BALANCED: Same damage rules (tougher but predictable)
        stats.maxHealth = 10000.0f;
        stats.attackDamage = 20.0f;
        stats.attackRange = 60.0f;
        stats.attackCooldown = 2.2f;
        stats.detectionRange = 350.0f;
        stats.aggroRange = 250.0f;
        stats.moveSpeed = 30.0f;
        stats.scale = 0.6f;
        break;
    }

    return stats;
}

EnemyStats EnemyStats::getStatsForType(EnemyAIType aiType, EnemyTier tier) {
    EnemyStats stats;

    switch (aiType) {
    case EnemyAIType::MELEE:
        stats = { 100.0f, 20.0f, 40.0f, 1.2f, 250.0f, 180.0f, 40.0f, 0.3f };
        break;
    case EnemyAIType::RANGED:
        stats = { 100.0f, 20.0f, 150.0f, 2.0f, 300.0f, 200.0f, 30.0f, 0.3f };
        break;
    case EnemyAIType::TANK:
        stats = { 100.0f, 20.0f, 50.0f, 1.8f, 200.0f, 150.0f, 25.0f, 0.4f };
        break;
    case EnemyAIType::FAST:
        stats = { 100.0f, 20.0f, 30.0f, 0.8f, 280.0f, 200.0f, 60.0f, 0.25f };
        break;
    }

    float tierMultiplier = 1.0f;
    switch (tier) {
    case EnemyTier::TRASH_MOB:
        tierMultiplier = 1.0f;
        break;
    case EnemyTier::ELITE:
        tierMultiplier = 2.5f;
        stats.scale *= 1.3f;
        break;
    case EnemyTier::BOSS:
        tierMultiplier = 8.0f;
        stats.scale *= 2.0f;
        break;
    }

    stats.maxHealth *= tierMultiplier;
    stats.attackDamage *= tierMultiplier;

    return stats;
}

// ============================================================================

EnemyNPC::EnemyNPC(EnemyAIType type, EnemyTier tier, EnemyVariant variant)
    : NPC(NPCType::HOSTILE), state(EnemyState::IDLE),
    aiType(type), tier(tier), variant(variant), currentPatrolIndex(0),
    retreatThreshold(20.0f), isAttacking(false),
    attackAnimDuration(0.5f), attackAnimTimer(0.0f),
    idleTexture(nullptr), walkTexture(nullptr), attackTexture(nullptr),
    deathTexture(nullptr), currentAnimFrame(0), animTimer(0.0f),
    animFrameDuration(0.1f), idleFrameCount(20), walkFrameCount(20),
    attackFrameCount(25), deathFrameCount(25),
    deathAnimTimer(0.0f), deathAnimDuration(2.5f), deathAnimComplete(false),
    isTargeted(false), targetPulseTimer(0.0f) {

    this->type = EntityType::ENEMY;
    this->solid = true;

    // Load stats based on variant
    EnemyStats stats = EnemyStats::getStatsForVariant(variant);
    this->health = stats.maxHealth;
    this->maxHealth = stats.maxHealth;
    this->attackDamage = stats.attackDamage;
    this->attackRange = stats.attackRange;
    this->attackCooldown = Timer(stats.attackCooldown);
    this->detectionRange = stats.detectionRange;
    this->aggroRange = stats.aggroRange;
    this->chaseSpeed = stats.moveSpeed;
    this->scale = stats.scale;

    // Set sprite dimensions (8-directional sprites)
    this->spriteWidth = 180.0f;
    this->spriteHeight = 180.0f;

    // Physics
    this->friction = 500.0f;
    this->maxSpeedX = chaseSpeed;
    this->maxSpeedY = chaseSpeed;

    // Collision
    this->collider = Collider(20.0f * scale, 20.0f * scale, 30.0f * scale, 10.0f * scale, true);

    // Initialize patrol points
    patrolPoints.push_back(glm::vec2(0, 0));
}

void EnemyNPC::loadTextures(ResourceManager& rm) {
    std::string typePrefix;
    switch (variant) {
    case EnemyVariant::BEAST_MELEE: typePrefix = "beast"; break;
    case EnemyVariant::HALBERD_FIGHTER: typePrefix = "halberd"; break;
    case EnemyVariant::BOSS_MELEE: typePrefix = "boss"; break;
    }

    textureKey = typePrefix;

    idleTexture = rm.getTexture(typePrefix + "_idle");
    walkTexture = rm.getTexture(typePrefix + "_walk");
    attackTexture = rm.getTexture(typePrefix + "_attack");
    deathTexture = rm.getTexture(typePrefix + "_death");

    texture = idleTexture ? idleTexture : rm.getTexture("enemy_placeholder");
}

void EnemyNPC::update(float deltaTime, GameState& gs) {
    // Death state handling
    if (state == EnemyState::DYING) {
        deathAnimTimer += deltaTime;
        updateAnimation(deltaTime);

        if (deathAnimTimer >= deathAnimDuration) {
            state = EnemyState::DEAD;
            active = false;  // Mark for cleanup
        }
        return;
    }

    if (health <= 0 && state != EnemyState::DYING) {
        texture = gs.getResourceManager().getTexture("beast_death");

        state = EnemyState::DYING;
        deathAnimTimer = 0.0f;
        currentAnimFrame = 0;
        velocity = glm::vec2(0, 0);
        playDeathAnimation();
        return;
    }

    if (state == EnemyState::DEAD) {
        active = false;
        return;
    }

    // Update attack animation timer
    if (isAttacking) {
        attackAnimTimer += deltaTime;
        if (attackAnimTimer >= attackAnimDuration) {
            isAttacking = false;
            attackAnimTimer = 0.0f;
        }
    }

    // Update target pulse
    if (isTargeted) {
        targetPulseTimer += deltaTime * 3.0f;
    }

    updateAI(deltaTime, gs);
    updateAnimation(deltaTime);
    applyMovement(deltaTime, maxSpeedX, maxSpeedY);
}

void EnemyNPC::updateAI(float deltaTime, GameState& gs) {
    Player* player = gs.getPlayer();
    if (!player) return;

    float distToPlayer = glm::distance(position, player->getPosition());

    switch (state) {
    case EnemyState::IDLE:
        texture = gs.getResourceManager().getTexture("beast_idle");

        playIdleAnimation();
        if (distToPlayer < detectionRange) {
            state = EnemyState::CHASING;
            setTarget(player);
        }
        break;

    case EnemyState::CHASING:
        texture = gs.getResourceManager().getTexture("beast_walk");

        playWalkAnimation();

        if (distToPlayer > detectionRange * 1.5f) {
            state = EnemyState::IDLE;

            velocity = glm::vec2(0, 0);
        }
        else if (distToPlayer <= attackRange) {
            state = EnemyState::ATTACKING;
        }
        else {
            glm::vec2 dir = glm::normalize(player->getPosition() - position);
            acceleration = dir * chaseSpeed * 10.0f;

            // Update direction for animation
            setDirection(dir.x, dir.y);
        }
        break;

    case EnemyState::ATTACKING:
        texture = gs.getResourceManager().getTexture("beast_attack");

        playAttackAnimation();
        attackCooldown.step(deltaTime);

        if (attackCooldown.isTimeout()) {
            performAttack(deltaTime);
            attackCooldown.reset();
        }

        if (distToPlayer > attackRange * 1.2f) {
            state = EnemyState::CHASING;
        }

        velocity = glm::vec2(0, 0);
        break;

    case EnemyState::PATROLLING:
        playWalkAnimation();
        patrol(deltaTime);
        break;

    case EnemyState::RETREATING:
        playWalkAnimation();
        if (health > maxHealth * (retreatThreshold / 100.0f) * 2.0f) {
            state = EnemyState::CHASING;
        }
        break;
    }
}

void EnemyNPC::performAttack(float deltaTime) {
    isAttacking = true;
    attackAnimTimer = 0.0f;

    glm::vec2 dirToTarget = glm::vec2(0, 0);
    if (target) {
        dirToTarget = glm::normalize(target->getPosition() - position);
    }

    float hitboxSize = attackRange * 0.9f;
    attackHitbox = {
        position.x + dirToTarget.x * (attackRange * 0.5f) - hitboxSize / 2,
        position.y + dirToTarget.y * (attackRange * 0.5f) - hitboxSize / 2,
        hitboxSize,
        hitboxSize
    };
}

SDL_FRect EnemyNPC::getAttackHitbox() const {
    return attackHitbox;
}

void EnemyNPC::patrol(float deltaTime) {
    if (patrolPoints.empty()) return;

    glm::vec2 targetPos = patrolPoints[currentPatrolIndex];
    if (glm::distance(position, targetPos) < 10.0f) {
        currentPatrolIndex = (currentPatrolIndex + 1) % patrolPoints.size();
    }
    else {
        glm::vec2 dir = glm::normalize(targetPos - position);
        acceleration = dir * (chaseSpeed * 0.5f) * 10.0f;
        setDirection(dir.x, dir.y);
    }
}

// ============================================================================
// ANIMATION SYSTEM (8-Directional Support)
// ============================================================================

void EnemyNPC::updateAnimation(float deltaTime) {
    animTimer += deltaTime;

    if (animTimer >= animFrameDuration) {
        animTimer = 0.0f;

        int maxFrames = idleFrameCount;

        if (state == EnemyState::CHASING || state == EnemyState::PATROLLING) {
            maxFrames = walkFrameCount;
        }
        else if (state == EnemyState::ATTACKING) {
            maxFrames = attackFrameCount;
        }
        else if (state == EnemyState::DYING) {
            maxFrames = deathFrameCount;

            // Death animation plays once, don't loop
            if (currentAnimFrame < maxFrames - 1) {
                currentAnimFrame++;
            }
            return;
        }

        currentAnimFrame = (currentAnimFrame + 1) % maxFrames;
    }

    // Update vertical sprite index based on direction
    if (velocity.x != 0 || velocity.y != 0) {
        verticalSpriteIndex = getDirectionIndex();
    }
}

void EnemyNPC::playIdleAnimation() {
    if (texture != idleTexture && idleTexture) {
        texture = idleTexture;
        currentAnimFrame = 0;
    }
}

void EnemyNPC::playWalkAnimation() {
    if (texture != walkTexture && walkTexture) {
        texture = walkTexture;
        currentAnimFrame = 0;
    }
}

void EnemyNPC::playAttackAnimation() {
    if (texture != attackTexture && attackTexture) {
        texture = attackTexture;
        currentAnimFrame = 0;
    }
}

void EnemyNPC::playDeathAnimation() {
    if (texture != deathTexture && deathTexture) {
        texture = deathTexture;
        currentAnimFrame = 0;
        animTimer = 0.0f;
    }
}

// ============================================================================
// RENDERING (8-Directional Sprite Support)
// ============================================================================

void EnemyNPC::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    if (!texture) {
        // Fallback colored rectangle
        SDL_Color color;
        switch (variant) {
        case EnemyVariant::BEAST_MELEE: color = { 200, 50, 50, 255 }; break;
        case EnemyVariant::HALBERD_FIGHTER: color = { 255, 150, 0, 255 }; break;
        case EnemyVariant::BOSS_MELEE: color = { 150, 0, 200, 255 }; break;
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_FRect dst = {
            position.x - viewport.x,
            position.y - viewport.y,
            spriteWidth * scale,
            spriteHeight * scale
        };
        SDL_RenderFillRect(renderer, &dst);
    }
    else {
        // Calculate source rect for current animation frame (8-directional)
        float srcX = currentAnimFrame * spriteWidth;
        float srcY = verticalSpriteIndex * spriteHeight;  // Row based on direction

        SDL_FRect src = { srcX, srcY, spriteWidth, spriteHeight };
        SDL_FRect dst = {
            position.x - viewport.x,
            position.y - viewport.y,
            spriteWidth * scale,
            spriteHeight * scale
        };
        SDL_RenderTexture(renderer, texture, &src, &dst);
    }

    // Render target highlight if targeted (only when alive)
    if (isTargeted && state != EnemyState::DYING && state != EnemyState::DEAD) {
        renderTargetHighlight(renderer, viewport);
    }
}

void EnemyNPC::renderTargetHighlight(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    float pulseScale = 1.0f + 0.15f * sin(targetPulseTimer);
    float radius = (spriteWidth * scale * 0.6f) * pulseScale;

    glm::vec2 screenPos = {
        position.x - viewport.x + (spriteWidth * scale) / 2,
        position.y - viewport.y + (spriteHeight * scale) / 2
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);

    const int segments = 32;
    for (int i = 0; i < segments; ++i) {
        float angle1 = (i / (float)segments) * 2.0f * 3.14159f;
        float angle2 = ((i + 1) / (float)segments) * 2.0f * 3.14159f;

        float x1 = screenPos.x + cos(angle1) * radius;
        float y1 = screenPos.y + sin(angle1) * radius;
        float x2 = screenPos.x + cos(angle2) * radius;
        float y2 = screenPos.y + sin(angle2) * radius;

        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }

    // Draw arrows pointing to target
    float arrowDist = radius + 10.0f;
    float arrowSize = 8.0f;

    for (int i = 0; i < 4; ++i) {
        float angle = (i / 4.0f) * 2.0f * 3.14159f;
        float arrowX = screenPos.x + cos(angle) * arrowDist;
        float arrowY = screenPos.y + sin(angle) * arrowDist;

        float tipX = arrowX + cos(angle) * arrowSize;
        float tipY = arrowY + sin(angle) * arrowSize;
        float left1X = arrowX + cos(angle + 2.5f) * arrowSize * 0.5f;
        float left1Y = arrowY + sin(angle + 2.5f) * arrowSize * 0.5f;
        float left2X = arrowX + cos(angle - 2.5f) * arrowSize * 0.5f;
        float left2Y = arrowY + sin(angle - 2.5f) * arrowSize * 0.5f;

        SDL_RenderLine(renderer, tipX, tipY, left1X, left1Y);
        SDL_RenderLine(renderer, tipX, tipY, left2X, left2Y);
    }
}

void EnemyNPC::handleCollision(Entity* other) {
    // Basic collision handling
}

void EnemyNPC::onPlayerInteract(Player* player) {
    // Enemies don't interact peacefully
}

// ============================================================================
// DAMAGE HANDLING
// ============================================================================

void EnemyNPC::takeDamage(float damage) {
    if (state == EnemyState::DYING || state == EnemyState::DEAD) return;

    health -= damage;

    // Retreat behavior (disable for bosses)
    if (health <= maxHealth * (retreatThreshold / 100.0f) &&
        tier != EnemyTier::BOSS &&
        variant != EnemyVariant::BOSS_MELEE &&
        health > 0) {
        state = EnemyState::RETREATING;
    }

    std::cout << "[Enemy] Took " << damage << " damage. HP: "
        << health << "/" << maxHealth << std::endl;

    // Trigger death if HP reaches 0
    if (health <= 0) {
        health = 0;
        std::cout << "[Enemy] Died!" << std::endl;
    }
}

void EnemyNPC::attackTarget() {
    // Damage is applied via hitbox collision detection
}

bool EnemyNPC::isDead() const {
    return state == EnemyState::DEAD || health <= 0;
}