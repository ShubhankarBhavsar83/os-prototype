#pragma once
#include "Movable.h"

enum class NPCType {
    FRIENDLY,
    HOSTILE,
    NEUTRAL
};

class NPC : public Movable {
protected:
    NPCType npcType;
    Entity* target;
    float detectionRange;
    float aggroRange;

public:
    NPC(NPCType type)
        : Movable(), npcType(type), target(nullptr),
        detectionRange(200.0f), aggroRange(150.0f) {
    }

    virtual ~NPC() = default;

    // Pure virtual function - must be implemented by derived classes
    virtual void onPlayerInteract(class Player* player) = 0;

    // Moved from protected to public accessors
    void setTarget(Entity* newTarget) { target = newTarget; }
    Entity* getTarget() const { return target; }

    NPCType getNPCType() const { return npcType; }
    float getDetectionRange() const { return detectionRange; }
    float getAggroRange() const { return aggroRange; }
};