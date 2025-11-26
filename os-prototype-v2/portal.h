#pragma once
#include "Entity.h"
#include <string>
#include <iostream>

class Portal : public Entity {
private:
    std::string destinationLevel;
    bool isActive;
    float glowIntensity;
    float glowSpeed;
    float activationRange;

public:
    Portal(const std::string& destination = "")
        : Entity(), destinationLevel(destination), isActive(true),
        glowIntensity(0.0f), glowSpeed(2.0f), activationRange(50.0f) {

        type = EntityType::PORTAL;
        solid = false;
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 1.0f;
        collider = Collider(0.0f, 0.0f, 0.0f, 0.0f, false);
    }

    void update(float deltaTime, GameState& gs) override {
        // Animate glow effect
        glowIntensity += glowSpeed * deltaTime;
        if (glowIntensity > 1.0f) glowIntensity = 0.0f;

        // Check player proximity for visual feedback
        Player* player = gs.getPlayer();
        if (player) {
            float dist = glm::distance(position, player->getPosition());
            if (dist < activationRange) {
                // Player is near - could show prompt
            }
        }
    }

    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override {
        if (texture) {
            SDL_FRect dst = {
                position.x - viewport.x,
                position.y - viewport.y,
                spriteWidth * scale,
                spriteHeight * scale
            };

            // Apply glow effect
            SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(200 + 55 * glowIntensity));
            SDL_RenderTexture(renderer, texture, nullptr, &dst);
            SDL_SetTextureAlphaMod(texture, 255);

            // Draw glow ring (optional visual)
            if (glowIntensity > 0.5f) {
                SDL_SetRenderDrawColor(renderer, 100, 150, 255,
                    static_cast<Uint8>(100 * glowIntensity));
                float glowRadius = (spriteWidth * scale * 0.6f) * (1.0f + glowIntensity * 0.2f);

                // Simple circle approximation
                for (int i = 0; i < 32; ++i) {
                    float angle = (i / 32.0f) * 2.0f * 3.14159f;
                    float nextAngle = ((i + 1) / 32.0f) * 2.0f * 3.14159f;

                    float x1 = dst.x + dst.w / 2 + cos(angle) * glowRadius;
                    float y1 = dst.y + dst.h / 2 + sin(angle) * glowRadius;
                    float x2 = dst.x + dst.w / 2 + cos(nextAngle) * glowRadius;
                    float y2 = dst.y + dst.h / 2 + sin(nextAngle) * glowRadius;

                    SDL_RenderLine(renderer, x1, y1, x2, y2);
                }
            }
        }
    }

    void handleCollision(Entity* other) override {
        // Collision handled by player interaction
    }

    void interact(class GameState* gs = nullptr) {
        if (!isActive || destinationLevel.empty()) return;

        std::cout << "Portal activated! Loading level: " << destinationLevel << std::endl;

        if (gs) {
            // Trigger level transition
            gs->loadLevel(destinationLevel);
        }
    }

    // Getters/Setters
    void setDestination(const std::string& dest) { destinationLevel = dest; }
    std::string getDestination() const { return destinationLevel; }
    void setActive(bool active) { isActive = active; }
    bool getActive() const { return isActive; }
};

// Optional: Foreground visual layer (frame/effects that render above player)
class PortalForeground : public Entity {
public:
    PortalForeground() : Entity() {
        type = EntityType::PORTAL;
        solid = false;
        active = true;
        spriteWidth = 32.0f;
        spriteHeight = 32.0f;
        scale = 1.0f;
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