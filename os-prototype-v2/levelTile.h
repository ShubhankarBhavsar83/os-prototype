#pragma once
#include "Entity.h"

enum class TileType {
    FLOOR_DIRT,
    FLOOR_GRASS,
    FLOOR_STONE,
    WALL
};

class LevelTile : public Entity {
private:
    TileType tileType;

public:
    LevelTile(TileType type) : Entity(), tileType(type) {
        this->type = EntityType::LEVEL_TILE;
        solid = false;
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 1.0f;
        collider = Collider(0.0f, 0.0f, 0.0f, 0.0f, false);
    }

    void update(float deltaTime, GameState& gs) override {
        // Tiles don't update
    }

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
        // Tiles don't react to collisions
    }

    TileType getTileType() const { return tileType; }
};