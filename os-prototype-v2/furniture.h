#pragma once
#include "Entity.h"
#include "Player.h"

class Furniture : public Entity {
private:
    bool layerManagement;

public:
    Furniture() : Entity(), layerManagement(true) {
        type = EntityType::FURNITURE;
        solid = true;
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 1.0f;
        collider = Collider(0.0f, 0.0f, 0.0f, 0.0f, true);
    }

    void update(float deltaTime, class GameState& gs) override;
    void updateLayer(const glm::vec2& playerPos, class GameState& gs);

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
        // Furniture is static - no collision response
    }

    void setLayerManagement(bool enabled) { layerManagement = enabled; }
};