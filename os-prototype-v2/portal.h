#pragma once
#include "Entity.h"
#include <string>
#include <iostream>
#include <map>
#include "glm/glm.hpp"
#include "player.h"
#include "gameState.h"

class Portal : public Entity {
private:
    std::string destinationLevel;
    bool isActive;
    float glowIntensity;
    float glowSpeed;
    float activationRange;

    // NEW: Level transition mapping system
    static std::map<std::string, std::string> levelTransitions;
    static bool transitionsInitialized;

    static void initializeLevelTransitions() {
        if (transitionsInitialized) return;

        // ========================================================================
        // LEVEL TRANSITION CONFIGURATION
        // ========================================================================
        // Define your level progression here:
        // Format: levelTransitions["current_level"] = "next_level";

        // TODO: Replace these with your actual level names
        levelTransitions["level_1"] = "level_2";   // Level 1 -> Level 2
        levelTransitions["level_2"] = "level_3";   // Level 2 -> Level 3
        levelTransitions["level_3"] = "level_1";   // Level 3 -> Level 1 (loop back)

        // Example with actual level names:
        levelTransitions["abyss"] = "forest";      // Abyss -> Forest
        levelTransitions["forest"] = "castle";     // Forest -> Castle
        levelTransitions["castle"] = "abyss";      // Castle -> Abyss (loop)

        transitionsInitialized = true;
        std::cout << "[Portal] Level transitions initialized." << std::endl;
    }

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

        // Initialize level transitions on first portal creation
        initializeLevelTransitions();
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

            // Draw glow ring
            if (glowIntensity > 0.5f) {
                SDL_SetRenderDrawColor(renderer, 100, 150, 255,
                    static_cast<Uint8>(100 * glowIntensity));
                float glowRadius = (spriteWidth * scale * 0.6f) * (1.0f + glowIntensity * 0.2f);

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
        if (!isActive || !gs) return;

        std::string currentLevel = gs->getCurrentLevel();

        // Check if we have a defined transition for this level
        auto it = levelTransitions.find(currentLevel);

        if (it != levelTransitions.end()) {
            std::string nextLevel = it->second;
            std::cout << "[Portal] Transitioning from '" << currentLevel
                << "' to '" << nextLevel << "'" << std::endl;
            gs->transitionToLevel(nextLevel);
        }
        else {
            std::cout << "[Portal] WARNING: No transition defined for level '"
                << currentLevel << "'" << std::endl;
            std::cout << "[Portal] Available transitions:" << std::endl;
            for (const auto& pair : levelTransitions) {
                std::cout << "  - " << pair.first << " -> " << pair.second << std::endl;
            }
        }
    }

    // Getters/Setters
    void setDestination(const std::string& dest) { destinationLevel = dest; }
    std::string getDestination() const { return destinationLevel; }
    void setActive(bool active) { isActive = active; }
    bool getActive() const { return isActive; }

    // NEW: Static method to add custom level transitions at runtime
    static void addLevelTransition(const std::string& from, const std::string& to) {
        initializeLevelTransitions();
        levelTransitions[from] = to;
        std::cout << "[Portal] Added transition: " << from << " -> " << to << std::endl;
    }
};

// REMOVED: Static member initialization moved to a .cpp file
// These should be in a portal.cpp file, not in the header

// Optional: Foreground visual layer
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