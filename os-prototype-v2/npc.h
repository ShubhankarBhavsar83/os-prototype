#pragma once
#include "Movable.h"
#include "Timer.h"

enum class NPCType {
    FRIENDLY,
    HOSTILE
};

class NPC : public Movable {
protected:
    NPCType npcType;
    std::string name;
    float detectionRange;
    float aggroRange;
    Entity* target;
    Timer idleTimer;
    Timer actionTimer;

public:
    NPC(NPCType type);
    virtual ~NPC() = default;

    virtual void update(float deltaTime, class GameState& gs) override = 0;
    virtual void onPlayerInteract(class Player* player) = 0;

    NPCType getNPCType() const;
    void setTarget(Entity* newTarget);
    Entity* getTarget() const;
};