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

    Entity* owner;
    Entity* target;
    float damage;
    float lifetime;
    float maxLifetime;
    float speed;
    float homingStrength;

    Timer impactAnimTimer;
    bool hasHit;

    // Animation system (8-directional)
    SDL_Texture* projectileTexture;
    SDL_Texture* impactTexture;
    int currentAnimFrame;
    float animTimer;
    float animFrameDuration;
    int projectileFrameCount;
    int impactFrameCount;

public:
    Projectile(ProjectileType type, Entity* owner);

    void update(float deltaTime, class GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override;

    void setTarget(Entity* target);
    void setDamage(float dmg) { damage = dmg; }  // NEW: Damage setter
    void launch(const glm::vec2& direction);
    void onImpact(Entity* hitEntity);
    bool shouldRemove() const;

    // Animation and texture loading
    void updateAnimation(float deltaTime);
    void loadTextures(class ResourceManager& rm);
};