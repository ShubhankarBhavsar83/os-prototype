#pragma once
#include "Entity.h"
#include <cmath>

class Movable : public Entity {
protected:
    float maxSpeedX;
    float maxSpeedY;
    float friction;
    float directionHorizontal; // -1 left, 0 none, 1 right
    float directionVertical;   // -1 up, 0 none, 1 down

public:
    Movable()
        : Entity(), maxSpeedX(75.0f), maxSpeedY(75.0f),
        directionHorizontal(0), directionVertical(0), friction(0) {
    }

    virtual ~Movable() = default;

    void setFriction(float f) { friction = f; }

    virtual void applyMovement(float deltaTime, float maxX, float maxY) {

        velocity += acceleration * deltaTime;

        if (acceleration.x == 0 && acceleration.y == 0) {
            float speed = glm::length(velocity);
            if (speed > 0) {
                float drop = base_deceleration * deltaTime;

                float newSpeed = std::max(speed - drop, 0.0f);

                if (speed > 0.0001f) {
                    velocity *= newSpeed / speed; 
                }
                else {
                    velocity = glm::vec2(0, 0);
                }
            }
        }

        float speed = glm::length(velocity);
        if (speed > maxSpeedX) {
            velocity = glm::normalize(velocity) * maxSpeedX;
        }

        position += velocity * deltaTime;

    }




    virtual void clampVelocity(float deltaTime) {

    }

    // ... (Keep Setters/Getters) ...
    void setDirection(float horizontal, float vertical) {
        directionHorizontal = horizontal;
        directionVertical = vertical;
    }

    glm::vec2 getDirection() const {
        return glm::vec2(directionHorizontal, directionVertical);
    }

    int getDirectionIndex() const {
        if (directionHorizontal > 0 && directionVertical == 0) return 0;
        else if (directionHorizontal > 0 && directionVertical > 0) return 1;
        else if (directionHorizontal == 0 && directionVertical > 0) return 2;
        else if (directionHorizontal < 0 && directionVertical > 0) return 3;
        else if (directionHorizontal < 0 && directionVertical == 0) return 4;
        else if (directionHorizontal < 0 && directionVertical < 0) return 5;
        else if (directionHorizontal == 0 && directionVertical < 0) return 6;
        else if (directionHorizontal > 0 && directionVertical < 0) return 7;
        return 0;
    }

    float getDirectionHorizontal() const { return directionHorizontal; }
    float getDirectionVertical() const { return directionVertical; }
};

