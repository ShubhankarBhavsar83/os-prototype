#pragma once
#include "NPC.h"
#include <string>
#include <queue>
#include <functional>

struct ChatMessage {
    std::string sender; // "player" or "npc"
    std::string text;
    uint64_t timestamp;
};

// NEW: Interaction types for polymorphic behavior
enum class InteractionType {
    CHAT_API,  // Calls n8n API
    QUEST_GIVE,
    SHOP,
    LORE
};

class FriendlyNPC : public NPC {
private:
    InteractionType interactionType;
    std::string npcName;
    std::string apiEndpoint; // e.g., "https://your-n8n-instance.com/webhook/chat"
    std::string conversationContext;
    std::queue<ChatMessage> chatHistory;

    bool chatActive;
    Timer responseWaitTimer;
    std::string pendingResponse;
    bool waitingForAPI;

    // Visual indicator
    float interactPromptAlpha;
    bool showInteractPrompt;

public:
    FriendlyNPC(InteractionType type = InteractionType::CHAT_API,
        const std::string& name = "Friendly NPC");

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

    // NEW: API integration (implement in .cpp with actual HTTP library)
    void callChatAPI(const std::string& userMessage);
    void processAPIResponse(const std::string& jsonResponse);

    // NEW: Polymorphic interaction
    void performInteraction(class Player* player);

    // Getters
    InteractionType getInteractionType() const { return interactionType; }
    void setAPIEndpoint(const std::string& endpoint) { apiEndpoint = endpoint; }
    void setShowPrompt(bool show) { showInteractPrompt = show; }
};