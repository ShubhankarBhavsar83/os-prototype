#include "FriendlyNpc.h"
#include "GameState.h"
#include "Player.h"
#include <iostream>

FriendlyNPC::FriendlyNPC(InteractionType type, const std::string& name)
    : NPC(NPCType::FRIENDLY),
    interactionType(type),  // Initialize with the passed argument
    npcName(name),          // Initialize with the passed argument
    chatActive(false),
    responseWaitTimer(0.0f)
{
    this->type = EntityType::FRIENDLY_NPC;
    this->solid = true;
    this->spriteWidth = 32.0f;
    this->spriteHeight = 32.0f;
    this->friction = 900.0f;

    // Initialize prompt visibility default
    this->showInteractPrompt = false;
}
void FriendlyNPC::update(float deltaTime, GameState& gs) {
    // Physics
    applyMovement(deltaTime, maxSpeedX, maxSpeedY);

    // Timer logic for chat API would go here
}

void FriendlyNPC::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    // Green rectangle fallback
    if (!texture) {
        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
        SDL_FRect dst = { position.x - viewport.x, position.y - viewport.y, spriteWidth * scale, spriteHeight * scale };
        SDL_RenderFillRect(renderer, &dst);
    }
    else {
        SDL_FRect dst = { position.x - viewport.x, position.y - viewport.y, spriteWidth * scale, spriteHeight * scale };
        SDL_RenderTexture(renderer, texture, nullptr, &dst);
    }

    if (chatActive) {
        renderChatUI(renderer);
    }
}

void FriendlyNPC::handleCollision(Entity* other) {
    // Stop moving on collision
}

void FriendlyNPC::onPlayerInteract(Player* player) {
    if (!chatActive) {
        startChat();
    }
    else {
        endChat();
    }
}

void FriendlyNPC::startChat() {
    chatActive = true;
    std::cout << "NPC: Hello traveler!" << std::endl;
}

void FriendlyNPC::endChat() {
    chatActive = false;
}

void FriendlyNPC::sendMessage(const std::string& message) {
    // API Logic stub
}

void FriendlyNPC::receiveResponse(const std::string& response) {
    // Store response
}

void FriendlyNPC::renderChatUI(SDL_Renderer* renderer) {
    // Draw simple bubble
}

bool FriendlyNPC::isChatActive() const {
    return chatActive;
}

void FriendlyNPC::callChatAPI(const std::string& userMessage) {}
void FriendlyNPC::processAPIResponse(const std::string& jsonResponse) {}