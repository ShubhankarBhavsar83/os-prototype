#include "Player.h"
#include "GameState.h"
#include "EnemyNpc.h"
#include "FriendlyNpc.h"
#include "Portal.h"
#include "Projectile.h"
#include <iostream>
#include <algorithm>

Player::Player()
    : Movable(), state(PlayerState::IDLE),
    dashSpeed(150.0f), dashDuration(0), dashDurationMax(700),
    dashCooldown(5000), dashCooldownMark(0), canDash(true),
    targetEntity(nullptr), currentTargetIndex(-1), targetingRange(400.0f),
    targetingMode(TargetingMode::NEAREST), interactableNearby(nullptr),
    interactionRange(60.0f), directionIndex(2),
    meleeCooldown(0.6f), meleeRange(50.0f), meleeDamage(25.0f),
    isMeleeAttacking(false), meleeAnimDuration(0.4f), meleeAnimTimer(0.0f),
    fireballCooldown(2.0f), fireballDamage(40.0f), canCastFireball(true),
    health(100.0f), maxHealth(100.0f) {

    type = EntityType::PLAYER;
    id = 100;
    spriteWidth = 128.0f;
    spriteHeight = 128.0f;
    scale = 0.3f;
    solid = true;

    base_acceleration = 700.0f;
    base_deceleration = 5000.0f;
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

    // Update attack cooldowns
    meleeCooldown.step(deltaTime);
    fireballCooldown.step(deltaTime);

    // Update melee attack animation
    if (isMeleeAttacking) {
        meleeAnimTimer += deltaTime;
        if (meleeAnimTimer >= meleeAnimDuration) {
            isMeleeAttacking = false;
            meleeAnimTimer = 0.0f;
            state = PlayerState::IDLE;
        }
    }

    // Update targeting (NEW: Now properly finds and highlights enemies)
    updateTargeting(gs);

    // Check for nearby interactables
    checkForInteractables(gs);

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
        applyMovement(deltaTime, maxSpeedX, maxSpeedY);
        break;
    }

    case PlayerState::DASHING: {
        texture = gs.getResourceManager().getTexture("player_roll");
        if (currentAnimation != 2) {
            playAnimation(2);
        }

        if (dashCooldownMark != 0) {
            if (dashDuration > dashDurationMax) {
                state = PlayerState::IDLE;
                canDash = false;
            }
            else {
                velocity = glm::vec2(directionHorizontal * dashSpeed,
                    directionVertical * dashSpeed);
                position += velocity * deltaTime;
            }
        }
        break;
    }

    case PlayerState::ATTACKING_MELEE: {
        velocity = glm::vec2(0, 0);
        break;
    }

    case PlayerState::ATTACKING_RANGED: {
        state = PlayerState::IDLE;
        break;
    }
    }

    stepAnimation(deltaTime);
    if (acceleration.x != 0 || acceleration.y != 0) {
        verticalSpriteIndex = getDirectionIndex();
    }

    if (isMeleeAttacking) {
        updateAttackHitbox();
    }
}

void Player::updateAttackHitbox() {
    glm::vec2 forward(directionHorizontal, directionVertical);
    if (glm::length(forward) > 0.01f) {
        forward = glm::normalize(forward);
    }

    float hitboxSize = meleeRange * 0.8f;
    meleeHitbox = {
        position.x + forward.x * (meleeRange * 0.7f) - hitboxSize / 2,
        position.y + forward.y * (meleeRange * 0.7f) - hitboxSize / 2,
        hitboxSize,
        hitboxSize
    };
}

void Player::meleeAttack(GameState& gs) {
    if (isMeleeAttacking || !meleeCooldown.isTimeout()) return;

    state = PlayerState::ATTACKING_MELEE;
    isMeleeAttacking = true;
    meleeAnimTimer = 0.0f;
    meleeCooldown.reset();

    updateAttackHitbox();

    std::cout << "Player performs melee attack!" << std::endl;
}

void Player::rangedAttack(GameState& gs) {
    if (!fireballCooldown.isTimeout() || !targetEntity) return;

    state = PlayerState::ATTACKING_RANGED;
    fireballCooldown.reset();

    // Create fireball projectile
    auto fireball = std::make_unique<Projectile>(ProjectileType::FIREBALL, this);
    fireball->setPosition(position);
    fireball->setTarget(targetEntity);

    glm::vec2 direction = glm::normalize(targetEntity->getPosition() - position);
    fireball->launch(direction);

    // Add to projectiles layer
    gs.addEntity(std::move(fireball), LAYER_IDX_PROJECTILES);

    std::cout << "Player casts fireball at target!" << std::endl;
}

// ============================================================================
// TARGETING SYSTEM (UPDATED)
// ============================================================================

void Player::updateTargeting(GameState& gs) {
    // Find all enemies in range
    findEnemiesInRange(gs);

    // Clear previous target highlight
    if (targetEntity && targetEntity->getType() == EntityType::ENEMY) {
        EnemyNPC* previousTarget = static_cast<EnemyNPC*>(targetEntity);
        previousTarget->setTargeted(false);
    }

    // Validate current target
    if (targetEntity) {
        bool targetValid = false;
        for (Entity* enemy : enemiesInRange) {
            if (enemy == targetEntity) {
                EnemyNPC* enemyNPC = static_cast<EnemyNPC*>(enemy);
                if (!enemyNPC->isDead()) {
                    targetValid = true;
                    break;
                }
            }
        }

        if (!targetValid) {
            clearTarget();
        }
    }

    // Set target highlight
    if (targetEntity && targetEntity->getType() == EntityType::ENEMY) {
        EnemyNPC* currentTarget = static_cast<EnemyNPC*>(targetEntity);
        currentTarget->setTargeted(true);
    }
}

void Player::findEnemiesInRange(GameState& gs) {
    enemiesInRange.clear();

    // Get enemies from game state
    std::vector<Entity*> enemies = gs.getEnemiesInRange(position, targetingRange);

    for (Entity* enemy : enemies) {
        enemiesInRange.push_back(enemy);
    }

    // Sort by distance if using NEAREST mode
    if (targetingMode == TargetingMode::NEAREST && !enemiesInRange.empty()) {
        std::sort(enemiesInRange.begin(), enemiesInRange.end(),
            [this](Entity* a, Entity* b) {
                return glm::distance(position, a->getPosition()) <
                    glm::distance(position, b->getPosition());
            });
    }
}

void Player::cycleTarget(GameState& gs) {
    if (enemiesInRange.empty()) {
        clearTarget();
        return;
    }

    // Clear previous target highlight
    if (targetEntity && targetEntity->getType() == EntityType::ENEMY) {
        EnemyNPC* previousTarget = static_cast<EnemyNPC*>(targetEntity);
        previousTarget->setTargeted(false);
    }

    // Cycle through available targets
    currentTargetIndex = (currentTargetIndex + 1) % enemiesInRange.size();
    targetEntity = enemiesInRange[currentTargetIndex];

    // Set new target highlight
    if (targetEntity && targetEntity->getType() == EntityType::ENEMY) {
        EnemyNPC* newTarget = static_cast<EnemyNPC*>(targetEntity);
        newTarget->setTargeted(true);
    }

    std::cout << "Target cycled to enemy " << currentTargetIndex << std::endl;
}

Entity* Player::findNearestEnemy(GameState& gs) {
    Entity* nearest = nullptr;
    float minDist = targetingRange;

    for (Entity* enemy : enemiesInRange) {
        float dist = glm::distance(position, enemy->getPosition());
        if (dist < minDist) {
            minDist = dist;
            nearest = enemy;
        }
    }

    return nearest;
}

void Player::clearTarget() {
    // Clear target highlight
    if (targetEntity && targetEntity->getType() == EntityType::ENEMY) {
        EnemyNPC* previousTarget = static_cast<EnemyNPC*>(targetEntity);
        previousTarget->setTargeted(false);
    }

    targetEntity = nullptr;
    currentTargetIndex = -1;
}

// ============================================================================

void Player::checkForInteractables(GameState& gs) {
    interactableNearby = nullptr;

    // Check for friendly NPCs
    std::vector<Entity*> characters = gs.getEntitiesInRange(position, interactionRange, LAYER_IDX_CHARACTERS);
    for (Entity* entity : characters) {
        if (entity->getType() == EntityType::FRIENDLY_NPC) {
            interactableNearby = entity;
            return;
        }
    }

    // Check for portals
    std::vector<Entity*> portals = gs.getEntitiesInRange(position, interactionRange, LAYER_IDX_PORTAL_BACKGROUND);
    for (Entity* entity : portals) {
        if (entity->getType() == EntityType::PORTAL) {
            interactableNearby = entity;
            return;
        }
    }
}

void Player::interact(GameState& gs) {
    if (!interactableNearby) return;

    switch (interactableNearby->getType()) {
    case EntityType::FRIENDLY_NPC: {
        FriendlyNPC* npc = static_cast<FriendlyNPC*>(interactableNearby);
        npc->onPlayerInteract(this);
        std::cout << "Interacting with Friendly NPC" << std::endl;
        break;
    }

    case EntityType::PORTAL: {
        Portal* portal = static_cast<Portal*>(interactableNearby);
        portal->interact(&gs);
        std::cout << "Using Portal" << std::endl;
        break;
    }

    default:
        break;
    }
}

void Player::takeDamage(float damage) {
    health -= damage;
    if (health < 0) health = 0;

    std::cout << "Player took " << damage << " damage. HP: " << health << "/" << maxHealth << std::endl;
}

void Player::heal(float amount) {
    health += amount;
    if (health > maxHealth) health = maxHealth;
}

void Player::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    if (!texture) return;

    float srcX = currentAnimation != -1 ? animations[currentAnimation].currentFrame() * spriteWidth : 0.0f;
    float srcY = verticalSpriteIndex * spriteHeight;

    SDL_FRect src = { srcX, srcY, spriteWidth, spriteHeight };
    SDL_FRect dst = {
        position.x - viewport.x,
        position.y - viewport.y,
        spriteWidth * scale,
        spriteHeight * scale
    };

    SDL_RenderTexture(renderer, texture, &src, &dst);

    // DEBUG: Draw targeting line to current target
    if (targetEntity) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderLine(renderer,
            position.x - viewport.x + (spriteWidth * scale) / 2,
            position.y - viewport.y + (spriteHeight * scale) / 2,
            targetEntity->getPosition().x - viewport.x,
            targetEntity->getPosition().y - viewport.y);
    }
}

void Player::handleCollision(Entity* other) {
    if (!other->isSolid()) return;

    SDL_FRect myRect = getBoundingBox();
    SDL_FRect otherRect = other->getBoundingBox();
    SDL_FRect intersection;

    if (SDL_GetRectIntersectionFloat(&myRect, &otherRect, &intersection)) {
        if (intersection.w < intersection.h) {
            if (velocity.x > 0) position.x -= intersection.w;
            else if (velocity.x < 0) position.x += intersection.w;
            velocity.x = 0;
        }
        else {
            if (velocity.y > 0) position.y -= intersection.h;
            else if (velocity.y < 0) position.y += intersection.h;
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

    // Movement
    if (keyState[SDL_SCANCODE_A]) movementDirectionSet.X -= 1.0f;
    if (keyState[SDL_SCANCODE_D]) movementDirectionSet.X += 1.0f;
    if (keyState[SDL_SCANCODE_W]) movementDirectionSet.Y -= 1.0f;
    if (keyState[SDL_SCANCODE_S]) movementDirectionSet.Y += 1.0f;

    setDirection(movementDirectionSet.X, movementDirectionSet.Y);

    if (movementDirectionSet.X != 0 || movementDirectionSet.Y != 0) {
        glm::vec2 inputDir = glm::normalize(glm::vec2(movementDirectionSet.X, movementDirectionSet.Y));
        current_acceleration = inputDir * base_acceleration;
        input = inputDir;
    }
    else {
        input = glm::vec2(0.0f, 0.0f);
    }

    acceleration = current_acceleration;

    // Dash
    if (keyState[SDL_SCANCODE_L] && canDash &&
        (movementDirectionSet.X != 0 || movementDirectionSet.Y != 0)) {
        startDash();
    }

    // Melee Attack
    if (keyState[SDL_SCANCODE_K]) {
        meleeAttack(gs);
    }

    // Ranged Attack
    if (keyState[SDL_SCANCODE_J]) {
        rangedAttack(gs);
    }
}

void Player::startDash() {
    if (!canDash) return;
    state = PlayerState::DASHING;
    dashCooldownMark = SDL_GetTicks();
}