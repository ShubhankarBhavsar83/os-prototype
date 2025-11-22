#pragma once
#include "NPC.h"
#include <string>
#include <queue>

struct ChatMessage {
    std::string sender; // "player" or "npc"
    std::string text;
    uint64_t timestamp;
};

class FriendlyNPC : public NPC {
private:
    std::string apiEndpoint;
    std::string conversationContext;
    std::queue<ChatMessage> chatHistory;
    bool chatActive;
    Timer responseWaitTimer;
    std::string pendingResponse;

public:
    FriendlyNPC();

    void update(float deltaTime, class GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override;
    void onPlayerInteract(class Player* player) override;

    // Chat system
    void startChat();
    void endChat();
    void sendMessage(const std::string& message);
    void receiveResponse(const std::string& response);
    void renderChatUI(SDL_Renderer* renderer);
    bool isChatActive() const;

    // API integration
    void callChatAPI(const std::string& userMessage);
    void processAPIResponse(const std::string& jsonResponse);
};