#pragma once
#include "Movable.h"
#include "Timer.h"
#include <string>

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

    // NPC visual representation
    std::string npcImageKey;  // Texture key for this NPC (e.g., "npc_marcus")

public:
    NPC(NPCType type);
    virtual ~NPC() = default;

    // Pure virtual function - must be implemented by derived classes
    virtual void onPlayerInteract(class Player* player) = 0;

    // Accessors
    NPCType getNPCType() const;
    void setTarget(Entity* newTarget);
    Entity* getTarget() const;
    float getDetectionRange() const { return detectionRange; }
    float getAggroRange() const { return aggroRange; }

    // NEW: Image/Texture assignment
    void setNPCImage(const std::string& imageKey) { npcImageKey = imageKey; }
    std::string getNPCImage() const { return npcImageKey; }
};