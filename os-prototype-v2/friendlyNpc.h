#pragma once
#include "NPC.h"
#include "ApiClient.h"
#include "ChatSystem.h"
#include <string>
#include <vector>
#include <mutex>

class FriendlyNPC : public NPC {
private:
    ChatSystem chatSystem;

    std::string n8nWebhookUrl = "http://localhost:5678/webhook/c5b211d5-458b-4d5e-b88e-d5676ab93601";
    bool isWaitingForResponse;
    std::string currentInput;
    std::string lastResponse;

    std::string characterName;
    std::string playerID;

    // Thread safety for API responses
    std::vector<std::string> pendingResponses;
    std::mutex responseMutex;

public:
    static FriendlyNPC* activeChatNPC;

    FriendlyNPC();
    ~FriendlyNPC();

    void update(float deltaTime, class GameState& gs) override;
    void render(SDL_Renderer* renderer, const SDL_FRect& viewport) override;
    void handleCollision(Entity* other) override {}
    void onPlayerInteract(class Player* player) override;

    void startChat();
    void endChat();
    void handleTextInput(const std::string& text);
    void handleKeyDown(SDL_Keycode key);

    void sendToAPI(const std::string& message);
    void onAPIResponse(const std::string& response);

    // Texture loading
    void loadTextures(class ResourceManager& rm);

    // Getters and Setters
    void setCharacterName(const std::string& name) { characterName = name; }
    void setPlayerID(const std::string& id) { playerID = id; }
    std::string getCharacterName() const { return characterName; }

    void setScreenDimensions(int width, int height);
};