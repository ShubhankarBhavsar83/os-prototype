#include "FriendlyNpc.h"
#include "GameState.h"
#include "CoordinateSystem.h"
#include <iostream>

FriendlyNPC* FriendlyNPC::activeChatNPC = nullptr;

static APIClient globalApiClient;

FriendlyNPC::FriendlyNPC()
    : NPC(NPCType::FRIENDLY), isWaitingForResponse(false),
    currentInput(""), lastResponse("Hello! Press E to chat."), font(nullptr),
    characterName("elara"), playerID("001") {  // Initialize character variables

    this->type = EntityType::FRIENDLY_NPC;
    this->solid = true;
    this->spriteWidth = 32.0f;
    this->spriteHeight = 32.0f;
    this->friction = 900.0f;

    static bool apiStarted = false;
    if (!apiStarted) {
        globalApiClient.start();
        apiStarted = true;
    }

    // Try both font paths
    font = TTF_OpenFont("assets/font.ttf", 16);
    if (!font) {
        font = TTF_OpenFont("assets/font/font.ttf", 16);
        if (!font) {
            std::cout << "[NPC] Warning: Failed to load font" << std::endl;
        }
    }

    chatSystem.setMessageCallback([this](const std::string& msg) {
        this->sendToAPI(msg);
        });

    chatSystem.addMessage("NPC", "Hello! Press E to chat.");

    std::cout << "[FriendlyNPC] Created NPC at " << this << std::endl;
}

FriendlyNPC::~FriendlyNPC() {
    if (font) TTF_CloseFont(font);
    if (activeChatNPC == this) {
        activeChatNPC = nullptr;
    }
}

void FriendlyNPC::update(float deltaTime, GameState& gs) {
    applyMovement(deltaTime,75.0f,75.0f);
}

void FriendlyNPC::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    SDL_FRect dst = { position.x - viewport.x, position.y - viewport.y, spriteWidth * scale, spriteHeight * scale };
    if (texture) SDL_RenderTexture(renderer, texture, nullptr, &dst);
    else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &dst);
    }

    if (chatSystem.isActive()) {
        float screenX = dst.x + (dst.w / 2);
        float screenY = dst.y;
        chatSystem.setPosition(screenX, screenY);
        chatSystem.render(renderer);
    }
}

void FriendlyNPC::onPlayerInteract(Player* player) {
    std::cout << "[FriendlyNPC] onPlayerInteract called! Chat active: " << chatSystem.isActive() << std::endl;

    if (!chatSystem.isActive()) {
        std::cout << "[FriendlyNPC] Starting chat..." << std::endl;
        startChat();
    }
    else {
        std::cout << "[FriendlyNPC] Ending chat..." << std::endl;
        endChat();
    }
}

void FriendlyNPC::startChat() {
    std::cout << "[FriendlyNPC] startChat() called" << std::endl;
    std::cout << "[FriendlyNPC] Current activeChatNPC: " << activeChatNPC << std::endl;
    std::cout << "[FriendlyNPC] This NPC: " << this << std::endl;

    if (activeChatNPC && activeChatNPC != this) {
        std::cout << "[FriendlyNPC] Closing previous NPC's chat" << std::endl;
        activeChatNPC->endChat();
    }

    activeChatNPC = this;
    std::cout << "[FriendlyNPC] Set activeChatNPC to: " << activeChatNPC << std::endl;

    chatSystem.activate();

    std::cout << "[FriendlyNPC] Chat system activated. Is active: " << chatSystem.isActive() << std::endl;
}

void FriendlyNPC::endChat() {
    std::cout << "[FriendlyNPC] endChat() called" << std::endl;
    chatSystem.deactivate();
    if (activeChatNPC == this) {
        activeChatNPC = nullptr;
        std::cout << "[FriendlyNPC] Cleared activeChatNPC" << std::endl;
    }
}

void FriendlyNPC::handleTextInput(const std::string& text) {
    std::cout << "[FriendlyNPC] handleTextInput: '" << text << "'" << std::endl;
    chatSystem.handleTextInput(text);
}

void FriendlyNPC::handleKeyDown(SDL_Keycode key) {
    std::cout << "[FriendlyNPC] handleKeyDown: " << SDL_GetKeyName(key) << std::endl;
    chatSystem.handleKeyPress(key);
}

void FriendlyNPC::sendToAPI(const std::string& message) {
    std::cout << "[FriendlyNPC] Sending to API: " << message << std::endl;

    // Escape quotes in the message
    std::string escapedMessage = message;
    size_t pos = 0;
    while ((pos = escapedMessage.find("\"", pos)) != std::string::npos) {
        escapedMessage.replace(pos, 1, "\\\"");
        pos += 2;
    }

    // Format: { "characterName":"elara", "context":"message", "playerID":"001" }
    std::string payload = "{ \"characterName\":\"" + characterName +
        "\", \"context\":\"" + escapedMessage +
        "\", \"playerID\":\"" + playerID + "\" }";

    std::cout << "[FriendlyNPC] JSON Payload: " << payload << std::endl;

    globalApiClient.sendRequest(n8nWebhookUrl, payload, [this](const std::string& response) {
        this->onAPIResponse(response);
        });
}

void FriendlyNPC::onAPIResponse(const std::string& response) {
    std::cout << "[FriendlyNPC] Raw API Response: " << response << std::endl;

    // Parse JSON response to extract just the "output" field
    std::string cleanedResponse = response;

    // Look for {"output":"..."} pattern
    size_t outputStart = response.find("\"output\":\"");
    if (outputStart != std::string::npos) {
        outputStart += 11; // Move past "output":"
        size_t outputEnd = response.find("\"}", outputStart);
        if (outputEnd != std::string::npos) {
            cleanedResponse = response.substr(outputStart, outputEnd - outputStart);

            // Unescape any escaped quotes
            size_t pos = 0;
            while ((pos = cleanedResponse.find("\\\"", pos)) != std::string::npos) {
                cleanedResponse.replace(pos, 2, "\"");
                pos += 1;
            }
        }
    }

    std::cout << "[FriendlyNPC] Cleaned Response: " << cleanedResponse << std::endl;

    // Use character name instead of "NPC"
    chatSystem.addMessage(characterName, cleanedResponse);
    chatSystem.setWaitingForResponse(false);
}