#include "NPC.h"
#include "GameState.h"

NPC::NPC(NPCType type)
    : Movable(), npcType(type), detectionRange(300.0f),
    aggroRange(150.0f), target(nullptr), idleTimer(2.0f), actionTimer(1.0f) {
    // Base initialization
}

NPCType NPC::getNPCType() const {
    return npcType;
}

void NPC::setTarget(Entity* newTarget) {
    target = newTarget;
}

Entity* NPC::getTarget() const {
    return target;
}