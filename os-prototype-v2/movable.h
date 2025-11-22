#pragma once
#include "Entity.h"
#include <cmath>

class Movable : public Entity {
protected:
    float maxSpeedX;
    float maxSpeedY;
    float directionHorizontal; // -1 left, 0 none, 1 right
    float directionVertical;   // -1 up, 0 none, 1 down

public:
    Movable()
        : Entity(), maxSpeedX(100.0f), maxSpeedY(70.0f),
        directionHorizontal(0), directionVertical(0) {
    }

    virtual ~Movable() = default;

    virtual void applyMovement(float deltaTime) {
        velocity += acceleration * deltaTime;
        clampVelocity();
        position += velocity * deltaTime;
    }

    virtual void applyFriction(float deltaTime, float frictionFactor) {
        if (velocity.x != 0) {
            float frictionX = (velocity.x > 0 ? -1.0f : 1.0f) * frictionFactor * deltaTime;
            if (std::abs(velocity.x) < std::abs(frictionX)) {
                velocity.x = 0;
            }
            else {
                velocity.x += frictionX;
            }
        }

        if (velocity.y != 0) {
            float frictionY = (velocity.y > 0 ? -1.0f : 1.0f) * frictionFactor * deltaTime;
            if (std::abs(velocity.y) < std::abs(frictionY)) {
                velocity.y = 0;
            }
            else {
                velocity.y += frictionY;
            }
        }
    }

    virtual void clampVelocity() {
        float currentMaxSpeedX = maxSpeedX;
        float currentMaxSpeedY = maxSpeedY;

        // Diagonal movement adjustment
        if (directionHorizontal != 0 && directionVertical != 0) {
            currentMaxSpeedX *= 0.707f; // cos(45°)
            currentMaxSpeedY *= 0.707f;
        }

        if (std::abs(velocity.x) > currentMaxSpeedX) {
            velocity.x = (velocity.x > 0 ? 1.0f : -1.0f) * currentMaxSpeedX;
        }
        if (std::abs(velocity.y) > currentMaxSpeedY) {
            velocity.y = (velocity.y > 0 ? 1.0f : -1.0f) * currentMaxSpeedY;
        }
    }

    void setDirection(float horizontal, float vertical) {
        directionHorizontal = horizontal;
        directionVertical = vertical;
    }

    glm::vec2 getDirection() const {
        return glm::vec2(directionHorizontal, directionVertical);
    }

    int getDirectionIndex() const {
        // 8-directional: 0=right, 1=down-right, 2=down, 3=down-left, 
        //                4=left, 5=up-left, 6=up, 7=up-right
        if (directionHorizontal > 0 && directionVertical == 0) return 0;
        else if (directionHorizontal > 0 && directionVertical > 0) return 1;
        else if (directionHorizontal == 0 && directionVertical > 0) return 2;
        else if (directionHorizontal < 0 && directionVertical > 0) return 3;
        else if (directionHorizontal < 0 && directionVertical == 0) return 4;
        else if (directionHorizontal < 0 && directionVertical < 0) return 5;
        else if (directionHorizontal == 0 && directionVertical < 0) return 6;
        else if (directionHorizontal > 0 && directionVertical < 0) return 7;
        return 0; // Default
    }

    float getDirectionHorizontal() const { return directionHorizontal; }
    float getDirectionVertical() const { return directionVertical; }
};