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
        loadTexture("tile_dark_dirt_textured", "assets/map_assets/tile_006.png");
        loadTexture("tile_dirt", "assets/map_assets/tile_003.png");
        loadTexture("tile_light_dirt", "assets/map_assets/tile_000.png");
        loadTexture("tile_medium_dirt", "assets/map_assets/tile_005.png");
        loadTexture("tile_light_dirt_patterned", "assets/map_assets/tile_009.png");
        loadTexture("tile_light_dirt_patchy", "assets/map_assets/tile_007.png");
        loadTexture("tile_light_dirt_textured_subtle", "assets/map_assets/tile_002.png");
        loadTexture("tile_light_dirt_warm", "assets/map_assets/tile_004.png");
        loadTexture("tile_dark_dirt", "assets/map_assets/tile_008.png");
        loadTexture("tile_light_dirt_2", "assets/map_assets/tile_001.png");
        loadTexture("tile_dirt_block", "assets/map_assets/tile_010.png");
        loadTexture("tile_wood_planks", "assets/map_assets/tile_011.png");
        loadTexture("tile_wood_grain", "assets/map_assets/tile_012.png");
        loadTexture("tile_wood_planks_light", "assets/map_assets/tile_013.png");
        loadTexture("tile_chiseled_dirt", "assets/map_assets/tile_014.png");
        loadTexture("tile_wood_floor", "assets/map_assets/tile_015.png");
        loadTexture("tile_rough_dirt", "assets/map_assets/tile_016.png");
        loadTexture("tile_cracked_earth", "assets/map_assets/tile_017.png");
        loadTexture("tile_cobblestone_large", "assets/map_assets/tile_018.png");
        loadTexture("tile_dirt_with_leaves", "assets/map_assets/tile_019.png");
        loadTexture("tile_dirt_with_moss", "assets/map_assets/tile_020.png");
        loadTexture("tile_dark_soil", "assets/map_assets/tile_021.png");
        loadTexture("tile_grass_light_standard", "assets/map_assets/tile_022.png");
        loadTexture("tile_grass_light_standard_2", "assets/map_assets/tile_023.png");
        loadTexture("tile_grass_light_flat", "assets/map_assets/tile_024.png");
        loadTexture("tile_dry_ridged_dirt", "assets/map_assets/tile_025.png");
        loadTexture("tile_dirt_with_grass_patches", "assets/map_assets/tile_026.png");
        loadTexture("tile_dense_moss", "assets/map_assets/tile_027.png");
        loadTexture("tile_lush_foliage", "assets/map_assets/tile_028.png");
        loadTexture("tile_bush_top", "assets/map_assets/tile_029.png");
        loadTexture("tile_bush_dense_blocky", "assets/map_assets/tile_030.png");
        loadTexture("tile_bush_large_leaves", "assets/map_assets/tile_031.png");
        loadTexture("tile_foliage_rounded_clusters", "assets/map_assets/tile_032.png");
        loadTexture("tile_bush_darker_base", "assets/map_assets/tile_033.png");
        loadTexture("tile_bush_tall_top", "assets/map_assets/tile_034.png");
        loadTexture("tile_ground_cover_thick", "assets/map_assets/tile_035.png");
        loadTexture("tile_shrub_dark_dense", "assets/map_assets/tile_036.png");
        loadTexture("tile_grass_edge_dense_dark", "assets/map_assets/tile_037.png");
        loadTexture("tile_grass_edge_mixed_border", "assets/map_assets/tile_038.png");
        loadTexture("tile_grass_edge_bushy_fringe", "assets/map_assets/tile_039.png");
        loadTexture("tile_grass", "assets/map_assets/tile_040.png");
        loadTexture("tile_flower_red_orange", "assets/map_assets/tile_041.png");
        loadTexture("tile_root_with_berries", "assets/map_assets/tile_042.png");
        loadTexture("tile_bush_dark_foliage", "assets/map_assets/tile_043.png");
        loadTexture("tile_bush_pink_flowers", "assets/map_assets/tile_044.png");
        loadTexture("tile_swirling_moss", "assets/map_assets/tile_045.png");
        loadTexture("tile_flower_cluster_multi", "assets/map_assets/tile_046.png");
        loadTexture("tile_fallen_petals_gems", "assets/map_assets/tile_047.png");
        loadTexture("tile_wood_log_small", "assets/map_assets/tile_048.png");
        loadTexture("tile_wood_logs_rough", "assets/map_assets/tile_049.png");
        loadTexture("tile_log_on_grass", "assets/map_assets/tile_050.png");
        loadTexture("tile_log_in_foliage", "assets/map_assets/tile_051.png");
        loadTexture("tile_hollow_stump", "assets/map_assets/tile_052.png");
        loadTexture("tile_small_mound_boulder", "assets/map_assets/tile_053.png");
        loadTexture("tile_rock_stack_medium", "assets/map_assets/tile_054.png");
        loadTexture("tile_rock_stack_tall", "assets/map_assets/tile_055.png");
        loadTexture("tile_rock_cluster_base", "assets/map_assets/tile_056.png");
        loadTexture("tile_rock_column_rugged", "assets/map_assets/tile_057.png");
        loadTexture("tile_scattered_rocks_small", "assets/map_assets/tile_058.png");
        loadTexture("tile_pillar", "assets/map_assets/tile_059.png");
        loadTexture("tile_red_stone_block", "assets/map_assets/tile_060.png");
        loadTexture("tile_cobblestone_stack", "assets/map_assets/tile_061.png");
        loadTexture("tile_angular_stones", "assets/map_assets/tile_062.png");
        loadTexture("tile_stone_platform_flat", "assets/map_assets/tile_063.png");
        loadTexture("tile_stone_pillar_rugged", "assets/map_assets/tile_064.png");
        loadTexture("tile_boulder_smooth", "assets/map_assets/tile_065.png");
        loadTexture("tile_rock_pile_water_edge", "assets/map_assets/tile_066.png");
        loadTexture("tile_rock_cluster_low", "assets/map_assets/tile_067.png");
        loadTexture("tile_stone_shard_jagged", "assets/map_assets/tile_068.png");
        loadTexture("tile_stone_platform_water", "assets/map_assets/tile_069.png");
        loadTexture("tile_stone_cluster_water_3", "assets/map_assets/tile_070.png");
        loadTexture("tile_boulders_submerged_2", "assets/map_assets/tile_071.png");
        loadTexture("tile_rock_outcrop_large_water", "assets/map_assets/tile_072.png");
        loadTexture("tile_stone_spire_water", "assets/map_assets/tile_073.png");
        loadTexture("tile_flat_rocks_overlapping_water", "assets/map_assets/tile_074.png");
        loadTexture("tile_stone_hex_small_water", "assets/map_assets/tile_075.png");
        loadTexture("tile_stone_block_water", "assets/map_assets/tile_076.png");
        loadTexture("tile_stone_path_cluster_water", "assets/map_assets/tile_077.png");
        loadTexture("tile_boulders_deep_water", "assets/map_assets/tile_078.png");
        loadTexture("tile_rock_outcrop_jagged_water", "assets/map_assets/tile_079.png");
        loadTexture("tile_stone_spire_water_2", "assets/map_assets/tile_080.png");
        loadTexture("tile_flat_rocks_rounded_water", "assets/map_assets/tile_081.png");
        loadTexture("tile_particle_scattered_a", "assets/map_assets/tile_082.png");
        loadTexture("tile_particle_cross_large", "assets/map_assets/tile_083.png");
        loadTexture("tile_particle_faint_cluster", "assets/map_assets/tile_084.png");
        loadTexture("tile_particle_splash_small", "assets/map_assets/tile_085.png");
        loadTexture("tile_ice_cracks_flow", "assets/map_assets/tile_086.png");
        loadTexture("tile_pool_depression_blue", "assets/map_assets/tile_087.png");
        loadTexture("tile_liquid_puddle_center", "assets/map_assets/tile_088.png");
        loadTexture("tile_liquid_ripples_active", "assets/map_assets/tile_089.png");
        loadTexture("tile_dark_ground_pool", "assets/map_assets/tile_090.png");
        loadTexture("tile_dark_ground_rough", "assets/map_assets/tile_091.png");
        loadTexture("tile_dark_ground_solid", "assets/map_assets/tile_092.png");
        loadTexture("tile_dark_ground_scattered_a", "assets/map_assets/tile_093.png");
        loadTexture("tile_dark_ground_scattered_b", "assets/map_assets/tile_094.png");
        loadTexture("tile_dark_ground_flow_lines", "assets/map_assets/tile_095.png");
        loadTexture("tile_dark_ground_textured_pool", "assets/map_assets/tile_096.png");
        loadTexture("tile_dark_ground_puddle_clean", "assets/map_assets/tile_097.png");
        loadTexture("tile_dark_ground_ripples_concentric", "assets/map_assets/tile_098.png");
        loadTexture("tile_dark_ground_pool_subtle", "assets/map_assets/tile_099.png");
        loadTexture("tile_dark_ground_waves", "assets/map_assets/tile_100.png");
        loadTexture("tile_dark_ground_solid_b", "assets/map_assets/tile_101.png");
        loadTexture("tile_dark_ground_debris", "assets/map_assets/tile_102.png");
        loadTexture("tile_dark_ground_blotchy", "assets/map_assets/tile_103.png");
        loadTexture("tile_ice_water_solid", "assets/map_assets/tile_104.png");
        loadTexture("tile_ice_water_highlight", "assets/map_assets/tile_105.png");
        loadTexture("tile_ice_water_textured_a", "assets/map_assets/tile_106.png");
        loadTexture("tile_ice_water_clean", "assets/map_assets/tile_107.png");
        loadTexture("tile_ice_water_flow_sweep", "assets/map_assets/tile_108.png");
        loadTexture("tile_ice_water_subtle_shade", "assets/map_assets/tile_109.png");
        loadTexture("tile_ice_water_speckled", "assets/map_assets/tile_110.png");
        loadTexture("tile_ice_water_reflection", "assets/map_assets/tile_111.png");
        loadTexture("tile_ice_water_corner_light", "assets/map_assets/tile_112.png");
        loadTexture("tile_ice_water_rough_light", "assets/map_assets/tile_113.png");
        loadTexture("tile_ice_water_cracked", "assets/map_assets/tile_114.png");
		loadTexture("marcus", "assets/map_assets/tile_119.png");
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