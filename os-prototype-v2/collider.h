#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

struct Collider {
    float left;
    float right;
    float top;
    float bottom;
    bool isSolid;  // NEW: Track if this collider should block movement

    Collider() : left(0), right(0), top(0), bottom(0), isSolid(false) {}

    Collider(float left, float right, float top, float bottom, bool isSolid = true)
        : left(left), right(right), top(top), bottom(bottom), isSolid(isSolid) {
    }

    SDL_FRect toRect(const glm::vec2& position, float width, float height, float scale) const {
        float rectA_width = (width * scale) - (left + right);
        float rectA_height = (height * scale) - (top + bottom);

        return SDL_FRect{
            .x = position.x + left,
            .y = position.y + top + (rectA_height / 2),
            .w = rectA_width,
            .h = rectA_height - (rectA_height / 2)
        };
    }

    bool intersects(const Collider& other,
        const glm::vec2& posA, const glm::vec2& posB,
        float widthA, float heightA, float scaleA,
        float widthB, float heightB, float scaleB) const {

        // NEW: Only check collision if both colliders are solid
        if (!this->isSolid || !other.isSolid) {
            return false;
        }

        SDL_FRect rectA = this->toRect(posA, widthA, heightA, scaleA);
        SDL_FRect rectB = other.toRect(posB, widthB, heightB, scaleB);

        SDL_FRect intersection;
        return SDL_GetRectIntersectionFloat(&rectA, &rectB, &intersection);
    }
};