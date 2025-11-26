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
    ResourceManager() : renderer(renderer) {}

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
        // ====================================================================
        // PLAYER ASSETS
        // ====================================================================
        loadTexture("player_run", "assets/player_assets/Run.png");
        loadTexture("player_idle", "assets/player_assets/Idle.png");
        loadTexture("player_roll", "assets/player_assets/Rolling.png");

        // ====================================================================
        // ENEMY ASSETS
        // ====================================================================
        // Beast (Fast Melee)
        loadTexture("beast_idle", "assets/enemy_assets/beast_idle.png");
        loadTexture("beast_walk", "assets/enemy_assets/beast_walk.png");
        loadTexture("beast_attack", "assets/enemy_assets/beast_attack.png");

        // Halberd Fighter (Medium Melee)
        loadTexture("halberd_idle", "assets/enemy_assets/halberd_idle.png");
        loadTexture("halberd_walk", "assets/enemy_assets/halberd_walk.png");
        loadTexture("halberd_attack", "assets/enemy_assets/halberd_attack.png");

        // Boss (Heavy Melee)
        loadTexture("boss_idle", "assets/enemy_assets/boss_idle.png");
        loadTexture("boss_walk", "assets/enemy_assets/boss_walk.png");
        loadTexture("boss_attack", "assets/enemy_assets/boss_attack.png");

        // Generic enemy fallback
        loadTexture("enemy_placeholder", "assets/enemy_assets/enemy_placeholder.png");

        // ====================================================================
        // TILE ASSETS
        // ====================================================================
        loadTexture("tile_dirt", "assets/map_assets/tile_003.png");
        loadTexture("tile_grass", "assets/map_assets/tile_040.png");
        loadTexture("tile_pillar", "assets/map_assets/tile_059.png");

        // ====================================================================
        // PROJECTILE ASSETS
        // ====================================================================
        // Fireball
        loadTexture("fireball", "assets/projectile_assets/fireball.png");
        loadTexture("fireball_impact", "assets/projectile_assets/fireball_impact.png");

        // Arrow
        loadTexture("arrow", "assets/projectile_assets/arrow.png");
        loadTexture("arrow_impact", "assets/projectile_assets/arrow_impact.png");

        // Magic Bolt
        loadTexture("magic_bolt", "assets/projectile_assets/magic_bolt.png");
        loadTexture("magic_bolt_impact", "assets/projectile_assets/magic_bolt_impact.png");

        // ====================================================================
        // ANIMATION SETS
        // ====================================================================
        const int ANIM_PLAYER_RUN = 0;
        const int ANIM_PLAYER_IDLE = 1;
        const int ANIM_PLAYER_ROLL = 2;

        const int ANIM_ENEMY_IDLE = 0;
        const int ANIM_ENEMY_RUN = 1;
        const int ANIM_ENEMY_ATTACK = 2;

        std::vector<Animation> playerAnims;
        std::vector<Animation> enemyAnims;

        playerAnims.resize(5);
        enemyAnims.resize(5);

        // Player animations
        playerAnims[ANIM_PLAYER_RUN] = Animation(15, 0.9f);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(15, 0.9f);
        playerAnims[ANIM_PLAYER_ROLL] = Animation(15, 0.9f);

        // Enemy animations (you can adjust frame counts and durations)
        enemyAnims[ANIM_ENEMY_IDLE] = Animation(4, 0.6f);
        enemyAnims[ANIM_ENEMY_RUN] = Animation(8, 0.8f);
        enemyAnims[ANIM_ENEMY_ATTACK] = Animation(6, 0.5f);

        createAnimationSet("player", playerAnims);
        createAnimationSet("enemy", enemyAnims);
    }

    void unloadAll() {
        for (auto& pair : textures) {
            SDL_DestroyTexture(pair.second);
        }
        textures.clear();
        animationSets.clear();
    }
};