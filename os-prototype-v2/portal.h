#pragma once
#include "Entity.h"
#include <iostream>

// The Main Portal (Background + Logic)
class Portal : public Entity {
public:
    Portal() : Entity() {
        type = EntityType::PORTAL;
        solid = false; // Trigger only
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 1.0f;

        // Collision box for triggering teleport
        collider = Collider(5.0f, 5.0f, 5.0f, 5.0f);
    }

    void update(float deltaTime, GameState& gs) override {
        // Logic for portal animation or checking player distance could go here
    }

    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override {
        if (texture) {
            SDL_FRect dst = {
                position.x - viewport.x,
                position.y - viewport.y,
                spriteWidth * scale,
                spriteHeight * scale
            };
            SDL_RenderTexture(renderer, texture, nullptr, &dst);
        }
    }

    void handleCollision(Entity* other) override {
        // Interaction handled by Player collision check
    }

    void interact() {
        std::cout << "PORTAL TRIGGERED! (Background Layer)" << std::endl;
    }
};

// The Frame (Foreground - Visual Only)
class PortalForeground : public Entity {
public:
    PortalForeground() : Entity() {
        type = EntityType::PORTAL; // Can share type or have a new one
        solid = false;
        active = true;
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 1.0f;
        // No collider needed for foreground visual
    }

    void update(float deltaTime, GameState& gs) override {
        // Passive visual
    }

    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override {
        if (texture) {
            SDL_FRect dst = {
                position.x - viewport.x,
                position.y - viewport.y,
                spriteWidth * scale,
                spriteHeight * scale
            };
            SDL_RenderTexture(renderer, texture, nullptr, &dst);
        }
    }

    void handleCollision(Entity* other) override {}
};