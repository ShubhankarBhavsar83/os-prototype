#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "Animation.h"

class ResourceManager {
private:
    SDL_Renderer* renderer;
    std::unordered_map<std::string, SDL_Texture*> textures;
    std::unordered_map<std::string, std::vector<Animation>> animationSets;

public:
    ResourceManager(SDL_Renderer* renderer) : renderer(renderer) {}

    ~ResourceManager() { unloadAll(); }

    SDL_Texture* loadTexture(const std::string& key, const std::string& filepath) {
        SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());

        if (tex) {
            SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
            textures[key] = tex;
        }
        else {
            SDL_Log("Failed to load texture: %s - Error: %s", filepath.c_str(), SDL_GetError());
        }
        return tex;
    }

    SDL_Texture* getTexture(const std::string& key) {
        auto it = textures.find(key);
        return (it != textures.end()) ? it->second : nullptr;
    }

    void createAnimationSet(const std::string& key, const std::vector<Animation>& anims) {
        animationSets[key] = anims;
    }

    std::vector<Animation> getAnimationSet(const std::string& key) {
        auto it = animationSets.find(key);
        return (it != animationSets.end()) ? it->second : std::vector<Animation>();
    }

    void loadAllAssets() {
        // Player
        loadTexture("player_run", "assets/player_assets/Run.png");
        loadTexture("player_idle", "assets/player_assets/Idle.png");
        loadTexture("player_roll", "assets/player_assets/Rolling.png");

        // Tiles
        loadTexture("tile_dirt", "assets/map_assets/tile_003.png");
        loadTexture("tile_grass", "assets/map_assets/tile_040.png");
        loadTexture("tile_pillar", "assets/map_assets/tile_059.png");

        // Projectiles
        loadTexture("fireball", "assets/projectile_assets/fireball_0.png");
        loadTexture("fireball_impact", "assets/projectile_assets/fireball_impact_100x100.png");

        // Create animation sets
        std::vector<Animation> playerAnims = {
            Animation(15, 1.2f),  // RUN
            Animation(15, 1.2f),  // IDLE
            Animation(15, 1.2f)   // ROLL
        };
        createAnimationSet("player", playerAnims);
    }

    void unloadAll() {
        for (auto& pair : textures) {
            SDL_DestroyTexture(pair.second);
        }
        textures.clear();
        animationSets.clear();
    }
};