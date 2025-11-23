#include "Furniture.h"
#include "GameState.h"

void Furniture::update(float deltaTime, GameState& gs) {
    // Check if we need to update layer based on player position
    Player* player = gs.getPlayer();
    if (player) {
        updateLayer(player->getPosition(), gs);
    }
}

void Furniture::updateLayer(const glm::vec2& playerPos, GameState& gs) {
    size_t newLayer;

    if (position.y > playerPos.y) {
        newLayer = LAYER_IDX_FURNITURE_FOREGROUND; // LAYER_IDX_FURNITURE_FOREGROUND
    }
    else {
        newLayer = LAYER_IDX_FURNITURE_BACKGROUND; // LAYER_IDX_FURNITURE_BACKGROUND
    }

    if (currentLayer != newLayer) {
        gs.moveEntityToLayer(this, newLayer);
    }
}