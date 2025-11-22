#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

struct Collider {
    float left;
    float right;
    float top;
    float bottom;

    Collider() : left(0), right(0), top(0), bottom(0) {}

    Collider(float left, float right, float top, float bottom)
        : left(left), right(right), top(top), bottom(bottom) {
    }

    SDL_FRect toRect(const glm::vec2& position, float width, float height, float scale) const {

        float rectA_width = (width * scale) - (left + right);
        float rectA_height = (height * scale) - (top + bottom);

        return SDL_FRect {
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

        SDL_FRect rectA = this->toRect(posA, widthA, heightA, scaleA);
        SDL_FRect rectB = other.toRect(posB, widthB, heightB, scaleB);

        SDL_FRect intersection;
        return SDL_GetRectIntersectionFloat(&rectA, &rectB, &intersection);
    }
};