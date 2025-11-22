#pragma once
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

class CoordinateSystem {
public:
    static glm::vec2 orthoToIso(int col, int row, int tileSize, int screenWidth, int screenHeight) {
        int tileWidth = tileSize;
        int tileHeight = tileSize / 2;

        float isoX = (col - row) * (tileWidth / 2.0f);
        float isoY = (col + row) * (tileHeight / 2.0f);

        float offsetX = screenWidth / 2.0f;
        float offsetY = screenHeight / (float)screenHeight;

        return glm::vec2(isoX + offsetX, isoY + offsetY);
    }

    static glm::vec2 screenToWorld(float screenX, float screenY, const SDL_FRect& viewport) {
        return glm::vec2(screenX + viewport.x, screenY + viewport.y);
    }

    static glm::vec2 worldToScreen(const glm::vec2& worldPos, const SDL_FRect& viewport) {
        return glm::vec2(worldPos.x - viewport.x, worldPos.y - viewport.y);
    }
};