#pragma once
#include "Entity.h"

class Furniture : public Entity {
public:
    Furniture() : Entity() {
        type = EntityType::FURNITURE;
        solid = true;
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 1.0f;

        collider = Collider(6.0f, 7.0f, 3.0f, 2.0f);
    }

    void update(float deltaTime, GameState& gs) override;

    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override {
        if (!texture) return;

        SDL_FRect dst = {
            position.x - viewport.x,
            position.y - viewport.y,
            spriteWidth * scale,
            spriteHeight * scale
        };

        SDL_RenderTexture(renderer, texture, nullptr, &dst);
    }

    void handleCollision(Entity* other) override {
        // Furniture is passive
    }

    void updateLayer(const glm::vec2& playerPos, GameState& gs);
};