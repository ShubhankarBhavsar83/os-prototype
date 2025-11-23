#pragma once
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <vector>
#include "Animation.h"
#include "Collider.h"
#include "resourceManager.h"

enum class EntityType {
    PLAYER,
    ENEMY,
    FRIENDLY_NPC,
    PROJECTILE,
    FURNITURE,
    LEVEL_TILE
};

class GameState; // Forward declaration

class Entity {
protected:
    int id;
    EntityType type;
    size_t currentLayer;

    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    glm::vec2 current_acceleration;
    glm::vec2 input;
    float base_acceleration;
    float base_deceleration;

    float spriteWidth;
    float spriteHeight;
    float scale;
    bool solid;
    bool active;
    Collider collider;

    int currentAnimation;
    int verticalSpriteIndex;

public:

    SDL_Texture* texture;
    std::vector<Animation> animations;

    Entity()
        : type(EntityType::LEVEL_TILE), id(0), currentLayer(0),
        position(0, 0), velocity(0, 0), acceleration(0, 0),
        spriteWidth(0), spriteHeight(0), scale(1.0f),
        solid(false), active(true), texture(nullptr),
        currentAnimation(-1), verticalSpriteIndex(0) {
    }



    virtual ~Entity() = default;

    // Pure virtual methods
    virtual void update(float deltaTime, GameState& gs) = 0;
    virtual void render(SDL_Renderer* renderer, const SDL_FRect& viewport) = 0;
    virtual void handleCollision(Entity* other) = 0;

    // Getters
    virtual int getId() const { return id; }
    virtual SDL_Texture getTexture() const { return *texture; }
    virtual std::vector<Animation> getAnimations() const { return animations; }
    virtual glm::vec2 getPosition() const { return position; }
    virtual EntityType getType() const { return type; }
    virtual bool isSolid() const { return solid; }
    virtual bool isActive() const { return active; }
    virtual int getID() const { return id; }
    virtual size_t getCurrentLayer() const { return currentLayer; }

    // Setters
    virtual void setId(int &entityId) {  id = entityId; }
    virtual void setTexture(SDL_Texture &tex) const { *texture = tex; }
    virtual void setAnimations(std::vector<Animation> ani) { animations = ani; }

    virtual void setPosition(const glm::vec2& pos) { position = pos; }
    virtual void setActive(bool act) { active = act; }
    virtual void setCurrentLayer(size_t layer) { currentLayer = layer; }

    // Collision
    virtual SDL_FRect getBoundingBox() const {
        return collider.toRect(position, spriteWidth, spriteHeight, scale);
    }

    // Animation
    void playAnimation(int animIndex) {
        if (animIndex >= 0 && animIndex < animations.size()) {
            currentAnimation = animIndex;
        }
    }

    void stepAnimation(float deltaTime) {
        if (currentAnimation >= 0 && currentAnimation < animations.size()) {
            animations[currentAnimation].step(deltaTime);
        }
    }
};