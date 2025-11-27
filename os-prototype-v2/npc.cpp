#pragma once
#include "Movable.h"
#include "Timer.h"

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
    Timer idleTimer;
    Timer actionTimer;

public:
    NPC(NPCType type);
    virtual ~NPC() = default;

    // Pure virtual function - must be implemented by derived classes
    virtual void onPlayerInteract(class Player* player) = 0;

    // Accessors (now match npc.cpp implementation)
    NPCType getNPCType() const;
    void setTarget(Entity* newTarget);
    Entity* getTarget() const;

    float getDetectionRange() const { return detectionRange; }
    float getAggroRange() const { return aggroRange; }
};