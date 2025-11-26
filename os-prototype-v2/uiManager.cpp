#include "UIManager.h"
#include "GameState.h"
#include <iostream>

UIManager::UIManager(SDL_Renderer* renderer)
    : renderer(renderer), iconSize(40.0f), iconSpacing(10.0f) {

    // Initialize player HP bar position (bottom left)
    playerHPBarBg = { 20, 280, 200, 20 };
    playerHPBarFill = { 22, 282, 196, 16 };

    // Load font (requires SDL_ttf)
    // font = TTF_OpenFont("assets/fonts/default.ttf", 16);
}

UIManager::~UIManager() {
    // if (font) TTF_CloseFont(font);
}

void UIManager::update(float deltaTime, GameState& gs) {
    // Update ability cooldowns (will be linked to player abilities)
    for (auto& icon : abilityIcons) {
        if (icon.onCooldown) {
            icon.cooldownCurrent -= deltaTime;
            if (icon.cooldownCurrent <= 0) {
                icon.onCooldown = false;
                icon.cooldownCurrent = 0;
            }
        }
    }

    // Update enemy nameplates
    updateNameplates(gs);
}

void UIManager::render(GameState& gs) {
    Player* player = gs.getPlayer();
    if (player) {
        renderPlayerHP(player);
    }

    renderAbilityIcons();
    renderNameplates(gs.getViewport());
}

void UIManager::renderPlayerHP(Player* player) {
    // Background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 200);
    SDL_RenderFillRect(renderer, &playerHPBarBg);

    // HP Fill
    renderHealthBar(playerHPBarFill, player->getHealthPercent(), true);

    // Border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &playerHPBarBg);

    // Text (requires SDL_ttf)
    // char hpText[32];
    // snprintf(hpText, sizeof(hpText), "%.0f / %.0f", 
    //          player->getHealth(), player->getMaxHealth());
    // SDL_Surface* surface = TTF_RenderText_Blended(font, hpText, {255,255,255,255});
    // SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    // SDL_RenderTexture(renderer, texture, nullptr, &textRect);
}

void UIManager::renderHealthBar(const SDL_FRect& bg, float healthPercent, bool isPlayer) {
    SDL_FRect fill = bg;
    fill.w *= healthPercent;

    // Color coding
    SDL_Color color;
    if (healthPercent > 0.6f) {
        color = isPlayer ? SDL_Color{ 0, 255, 0, 255 } : SDL_Color{ 255, 50, 50, 255 };
    }
    else if (healthPercent > 0.3f) {
        color = { 255, 200, 0, 255 };
    }
    else {
        color = { 255, 0, 0, 255 };
    }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &fill);
}

void UIManager::addAbilityIcon(const std::string& name, const std::string& texturePath, float cooldown) {
    AbilityIcon icon;
    icon.name = name;
    icon.cooldownMax = cooldown;
    icon.cooldownCurrent = 0;
    icon.onCooldown = false;

    // Position icons in bottom center
    float startX = 300;
    float y = 280;
    float xOffset = abilityIcons.size() * (iconSize + iconSpacing);

    icon.rect = { startX + xOffset, y, iconSize, iconSize };

    // Load texture (you'd do this through ResourceManager)
    // icon.iconTexture = ...;

    abilityIcons.push_back(icon);
}

void UIManager::updateAbilityCooldown(const std::string& name, float currentCooldown) {
    for (auto& icon : abilityIcons) {
        if (icon.name == name) {
            icon.cooldownCurrent = currentCooldown;
            icon.onCooldown = (currentCooldown > 0);
            break;
        }
    }
}

void UIManager::renderAbilityIcons() {
    for (const auto& icon : abilityIcons) {
        // Background
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
        SDL_RenderFillRect(renderer, &icon.rect);

        // Icon texture
        if (icon.iconTexture) {
            SDL_RenderTexture(renderer, icon.iconTexture, nullptr, &icon.rect);
        }

        // Cooldown overlay
        if (icon.onCooldown) {
            renderCooldownOverlay(icon.rect, icon.getCooldownPercent());
        }

        // Border
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &icon.rect);
    }
}

void UIManager::renderCooldownOverlay(const SDL_FRect& rect, float cooldownPercent) {
    SDL_FRect overlay = rect;
    overlay.h *= cooldownPercent;
    overlay.y = rect.y + (rect.h - overlay.h);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &overlay);

    // Cooldown text
    // char cooldownText[16];
    // snprintf(cooldownText, sizeof(cooldownText), "%.1f", cooldownPercent * cooldownMax);
}

void UIManager::updateNameplates(GameState& gs) {
    nameplates.clear();

    // Iterate through enemies and create nameplates
    // This requires GameState to expose layer iteration
    // Pseudocode:
    /*
    for (Entity* entity : gs.getEntitiesInLayer(LAYER_IDX_CHARACTERS)) {
        if (entity->getType() == EntityType::ENEMY) {
            EnemyNPC* enemy = static_cast<EnemyNPC*>(entity);
            if (!enemy->isDead()) {
                Nameplate np;
                np.enemy = entity;
                np.name = enemy->getTier() == EnemyTier::BOSS ? "BOSS" : "Enemy";

                // Position above enemy
                glm::vec2 pos = enemy->getPosition();
                float width = 60;
                float height = 8;
                np.hpBarBg = { pos.x - width/2, pos.y - 40, width, height };
                np.hpBarFill = { pos.x - width/2 + 1, pos.y - 39, width - 2, height - 2 };

                nameplates.push_back(np);
            }
        }
    }
    */
}

void UIManager::renderNameplates(const SDL_FRect& viewport) {
    for (const auto& np : nameplates) {
        EnemyNPC* enemy = static_cast<EnemyNPC*>(np.enemy);

        // Convert world position to screen position
        SDL_FRect screenBg = np.hpBarBg;
        screenBg.x -= viewport.x;
        screenBg.y -= viewport.y;

        SDL_FRect screenFill = np.hpBarFill;
        screenFill.x -= viewport.x;
        screenFill.y -= viewport.y;

        // Background
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 200);
        SDL_RenderFillRect(renderer, &screenBg);

        // HP bar
        renderHealthBar(screenFill, enemy->getHealthPercent(), false);

        // Border
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &screenBg);

  /*     Name text (requires SDL_ttf)
         SDL_Surface* nameSurface = TTF_RenderText_Blended(font, np.name.c_str(), {255,255,255,255});*/
    }
}