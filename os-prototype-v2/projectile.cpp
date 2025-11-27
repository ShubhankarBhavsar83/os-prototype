#include "Projectile.h"
#include "GameState.h"
#include "EnemyNpc.h"
#include <iostream>

Projectile::Projectile(ProjectileType type, Entity* owner)
    : Movable(), state(ProjectileState::MOVING), projectileType(type),
    owner(owner), target(nullptr), lifetime(0.0f), hasHit(false),
    impactAnimTimer(0.4f), currentAnimFrame(0), animTimer(0.0f),
    animFrameDuration(0.08f), projectileFrameCount(4), impactFrameCount(6) {

    this->type = EntityType::PROJECTILE;
    this->solid = false;

    // Set stats based on projectile type
    switch (type) {
    case ProjectileType::FIREBALL:
        damage = 40.0f;
        speed = 200.0f;
        maxLifetime = 3.0f;
        homingStrength = 0.3f;
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 0.8f;
        projectileFrameCount = 4;
        impactFrameCount = 6;
        break;

    case ProjectileType::ARROW:
        damage = 25.0f;
        speed = 350.0f;
        maxLifetime = 2.0f;
        homingStrength = 0.0f;
        spriteWidth = 16.0f;
        spriteHeight = 16.0f;
        scale = 1.0f;
        projectileFrameCount = 1;
        impactFrameCount = 4;
        break;

    case ProjectileType::MAGIC_BOLT:
        damage = 30.0f;
        speed = 250.0f;
        maxLifetime = 2.5f;
        homingStrength = 0.6f;
        spriteWidth = 24.0f;
        spriteHeight = 24.0f;
        scale = 1.0f;
        projectileFrameCount = 4;
        impactFrameCount = 5;
        break;
    }

    collider = Collider(5.0f, 5.0f, 5.0f, 5.0f);
}

void Projectile::launch(const glm::vec2& direction) {
    velocity = direction * speed;
    state = ProjectileState::MOVING;
}

void Projectile::setTarget(Entity* newTarget) {
    target = newTarget;
}

void Projectile::update(float deltaTime, GameState& gs) {
    lifetime += deltaTime;

    switch (state) {
    case ProjectileState::MOVING: {
        // CRITICAL FIX: Validate target before accessing it
        // Check if target is null, inactive, or an enemy that's dead
        bool targetValid = false;
        if (target && target->isActive()) {
            // Additional check for enemy targets - make sure they're not dead
            if (target->getType() == EntityType::ENEMY) {
                EnemyNPC* enemyTarget = static_cast<EnemyNPC*>(target);
                targetValid = !enemyTarget->isDead();
            }
            else {
                targetValid = true;
            }
        }

        // Clear invalid targets
        if (!targetValid) {
            target = nullptr;
        }

        // Apply homing if we have a valid target
        if (target && homingStrength > 0.0f) {
            glm::vec2 toTarget = target->getPosition() - position;
            float distToTarget = glm::length(toTarget);

            if (distToTarget > 5.0f) {
                glm::vec2 desiredVelocity = glm::normalize(toTarget) * speed;
                glm::vec2 steering = desiredVelocity - velocity;
                velocity += steering * homingStrength * deltaTime;

                // Maintain constant speed
                if (glm::length(velocity) > 0.01f) {
                    velocity = glm::normalize(velocity) * speed;
                }
            }
        }

        // Update position
        position += velocity * deltaTime;

        // Check lifetime
        if (lifetime >= maxLifetime) {
            active = false;
        }

        // Update animation
        updateAnimation(deltaTime);
        break;
    }

    case ProjectileState::IMPACTING: {
        impactAnimTimer.step(deltaTime);
        updateAnimation(deltaTime);

        if (impactAnimTimer.isTimeout()) {
            active = false;
        }
        break;
    }

    case ProjectileState::INACTIVE: {
        active = false;
        break;
    }
    }
}

// ============================================================================
// ANIMATION SYSTEM
// ============================================================================

void Projectile::updateAnimation(float deltaTime) {
    animTimer += deltaTime;

    if (animTimer >= animFrameDuration) {
        animTimer = 0.0f;

        int maxFrames = (state == ProjectileState::MOVING) ? projectileFrameCount : impactFrameCount;

        if (state == ProjectileState::IMPACTING) {
            // Impact animation plays once
            currentAnimFrame++;
            if (currentAnimFrame >= maxFrames) {
                currentAnimFrame = maxFrames - 1;
            }
        }
        else {
            // Projectile animation loops
            currentAnimFrame = (currentAnimFrame + 1) % maxFrames;
        }
    }
}

void Projectile::loadTextures(ResourceManager& rm) {
    std::string projectileKey;
    std::string impactKey;

    switch (projectileType) {
    case ProjectileType::FIREBALL:
        projectileKey = "fireball";
        impactKey = "fireball_impact";
        break;
    case ProjectileType::ARROW:
        projectileKey = "arrow";
        impactKey = "arrow_impact";
        break;
    case ProjectileType::MAGIC_BOLT:
        projectileKey = "magic_bolt";
        impactKey = "magic_bolt_impact";
        break;
    }

    projectileTexture = rm.getTexture(projectileKey);
    impactTexture = rm.getTexture(impactKey);

    // Use projectile texture as default
    texture = projectileTexture;
}

// ============================================================================
// RENDERING
// ============================================================================

void Projectile::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    if (state == ProjectileState::MOVING) {
        if (projectileTexture) {
            // ANIMATION IMPLEMENTATION:
            // Calculate source rect for current animation frame
            float srcX = currentAnimFrame * spriteWidth;
            float srcY = 0.0f;

            SDL_FRect src = { srcX, srcY, spriteWidth, spriteHeight };
            SDL_FRect dst = {
                position.x - viewport.x - (spriteWidth * scale) / 2,
                position.y - viewport.y - (spriteHeight * scale) / 2,
                spriteWidth * scale,
                spriteHeight * scale
            };

            // Calculate rotation angle from velocity
            float angle = atan2(velocity.y, velocity.x) * (180.0f / 3.14159f);

            SDL_RenderTextureRotated(renderer, projectileTexture, &src, &dst,
                angle, nullptr, SDL_FLIP_NONE);
        }
        else {
            // Fallback: colored square placeholder
            SDL_Color color;
            switch (projectileType) {
            case ProjectileType::FIREBALL: color = { 255, 100, 0, 255 }; break;
            case ProjectileType::ARROW: color = { 200, 200, 200, 255 }; break;
            case ProjectileType::MAGIC_BOLT: color = { 100, 100, 255, 255 }; break;
            }

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_FRect dst = {
                position.x - viewport.x - 5,
                position.y - viewport.y - 5,
                10, 10
            };
            SDL_RenderFillRect(renderer, &dst);
        }
    }
    else if (state == ProjectileState::IMPACTING) {
        if (impactTexture) {
            // ANIMATION IMPLEMENTATION:
            // Impact animation with multiple frames
            float srcX = currentAnimFrame * spriteWidth * 3.0f; // Impact sprites are larger
            float srcY = 0.0f;

            SDL_FRect src = { srcX, srcY, spriteWidth * 3.0f, spriteHeight * 3.0f };
            SDL_FRect dst = {
                position.x - viewport.x - (spriteWidth * scale * 1.5f),
                position.y - viewport.y - (spriteHeight * scale * 1.5f),
                spriteWidth * scale * 3.0f,
                spriteHeight * scale * 3.0f
            };

            SDL_RenderTexture(renderer, impactTexture, &src, &dst);
        }
        else {
            // Fallback: expanding circle
            float impactScale = 1.0f + (currentAnimFrame / (float)impactFrameCount) * 2.0f;
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 128);
            SDL_FRect dst = {
                position.x - viewport.x - 15 * impactScale,
                position.y - viewport.y - 15 * impactScale,
                30 * impactScale,
                30 * impactScale
            };
            SDL_RenderFillRect(renderer, &dst);
        }
    }
}

// ============================================================================
// COLLISION HANDLING
// ============================================================================

void Projectile::handleCollision(Entity* other) {
    if (hasHit || !other || other == owner) return;

    // SAFETY CHECK: Validate entity before accessing
    if (!other->isActive()) return;

    // Only collide with enemies if shot by player, or player if shot by enemy
    bool shouldCollide = false;

    if (owner->getType() == EntityType::PLAYER && other->getType() == EntityType::ENEMY) {
        shouldCollide = true;
    }
    else if (owner->getType() == EntityType::ENEMY && other->getType() == EntityType::PLAYER) {
        shouldCollide = true;
    }

    if (shouldCollide) {
        onImpact(other);
    }
}

void Projectile::onImpact(Entity* hitEntity) {
    // SAFETY CHECK: Validate entity
    if (!hitEntity || !hitEntity->isActive()) return;

    hasHit = true;
    state = ProjectileState::IMPACTING;
    velocity = glm::vec2(0, 0);
    currentAnimFrame = 0; // Reset for impact animation
    texture = impactTexture; // Switch to impact texture

    // Apply damage
    if (hitEntity->getType() == EntityType::ENEMY) {
        EnemyNPC* enemy = static_cast<EnemyNPC*>(hitEntity);
        enemy->takeDamage(damage);
        std::cout << "Projectile hit enemy for " << damage << " damage!" << std::endl;
    }
    else if (hitEntity->getType() == EntityType::PLAYER) {
        Player* player = static_cast<Player*>(hitEntity);
        player->takeDamage(damage);
        std::cout << "Projectile hit player for " << damage << " damage!" << std::endl;
    }
}

bool Projectile::shouldRemove() const {
    return !active;
}