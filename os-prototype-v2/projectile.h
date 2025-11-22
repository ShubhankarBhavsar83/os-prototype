#pragma once
#include "Movable.h"
#include "Timer.h"

enum class ProjectileState {
    MOVING,
    IMPACTING,
    INACTIVE
};

enum class ProjectileType {
    FIREBALL,
    ARROW,
    MAGIC_BOLT
};

class Projectile : public Movable {
private:
    ProjectileState state;
    ProjectileType projectileType;

    Entity* owner;      // Who fired it
    Entity* target;     // Homing target
    float damage;
    float lifetime;
    float maxLifetime;
    float speed;
    float homingStrength; // 0 = no homing, 1 = perfect tracking

    Timer impactAnimTimer;
    bool hasHit;

public:
    Projectile(ProjectileType type, Entity* owner);

    void update(float deltaTime, class GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override;

    void setTarget(Entity* target);
    void launch(const glm::vec2& direction);
    void onImpact(Entity* hitEntity);
    bool shouldRemove() const;
};