#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include "Player.h"
#include "EnemyNpc.h"

struct AbilityIcon {
    std::string name;
    SDL_Texture* iconTexture;
    float cooldownMax;
    float cooldownCurrent;
    SDL_FRect rect;
    bool onCooldown;

    float getCooldownPercent() const {
        return onCooldown ? (cooldownCurrent / cooldownMax) : 0.0f;
    }
};

class UIManager {
private:
    SDL_Renderer* renderer;

    // Player UI
    SDL_FRect playerHPBarBg;
    SDL_FRect playerHPBarFill;

    // Ability Icons
    std::vector<AbilityIcon> abilityIcons;
    float iconSize;
    float iconSpacing;

    // Enemy nameplates
    struct Nameplate {
        Entity* enemy;
        SDL_FRect hpBarBg;
        SDL_FRect hpBarFill;
        SDL_FRect nameRect;
        std::string name;
    };
    std::vector<Nameplate> nameplates;

    // Fonts (you'll need SDL_ttf)
    // TTF_Font* font;

public:
    UIManager(SDL_Renderer* renderer);
    ~UIManager();

    void update(float deltaTime, class GameState& gs);
    void render(class GameState& gs);

    // Player HP
    void renderPlayerHP(Player* player);

    // Abilities
    void addAbilityIcon(const std::string& name, const std::string& texturePath, float cooldown);
    void updateAbilityCooldown(const std::string& name, float currentCooldown);
    void renderAbilityIcons();

    // Enemy UI
    void updateNameplates(class GameState& gs);
    void renderNameplates(const SDL_FRect& viewport);

    // Helper rendering
    void renderHealthBar(const SDL_FRect& bg, float healthPercent, bool isPlayer = false);
    void renderCooldownOverlay(const SDL_FRect& rect, float cooldownPercent);
};