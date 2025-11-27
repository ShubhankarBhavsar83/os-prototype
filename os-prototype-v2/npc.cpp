#include "NPC.h"

NPC::NPC(NPCType type)
    : npcType(type), target(nullptr), detectionRange(0.0f), aggroRange(0.0f)
{
    // Initialize defaults common to all NPCs
    this->solid = true;
    this->friction = 500.0f;
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