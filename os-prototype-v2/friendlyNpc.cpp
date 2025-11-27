#include "FriendlyNpc.h"
#include "GameState.h"
#include "CoordinateSystem.h"
#include <iostream>

FriendlyNPC* FriendlyNPC::activeChatNPC = nullptr;

static APIClient globalApiClient;

FriendlyNPC::FriendlyNPC()
    : NPC(NPCType::FRIENDLY), isWaitingForResponse(false),
    currentInput(""), lastResponse("Hello! Press E to chat."),
    characterName("elara"), playerID("001") {

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

    chatSystem.setMessageCallback([this](const std::string& msg) {
        this->sendToAPI(msg);
        });

    chatSystem.addMessage("NPC", "Hello! Press E to chat.");

    std::cout << "[FriendlyNPC] Created NPC at " << this << std::endl;
}

FriendlyNPC::~FriendlyNPC() {
    if (activeChatNPC == this) {
        activeChatNPC = nullptr;
    }
    std::cout << "[FriendlyNPC] Destroyed NPC at " << this << std::endl;
}

void FriendlyNPC::update(float deltaTime, GameState& gs) {
    applyMovement(deltaTime, 75.0f, 75.0f);
}

void FriendlyNPC::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    // Render NPC sprite
    SDL_FRect dst = {
        position.x - viewport.x,
        position.y - viewport.y,
        spriteWidth * scale,
        spriteHeight * scale
    };

    if (texture) {
        SDL_RenderTexture(renderer, texture, nullptr, &dst);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &dst);
    }

    // Render chat system if active
    if (chatSystem.isActive()) {
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

    if (activeChatNPC && activeChatNPC != this) {
        std::cout << "[FriendlyNPC] Closing previous NPC's chat" << std::endl;
        activeChatNPC->endChat();
    }

    activeChatNPC = this;
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

    std::string payload = "{ \"characterName\":\"" + characterName +
        "\", \"context\":\"" + escapedMessage +
        "\", \"playerID\":\"" + playerID + "\" }";

    std::cout << "[FriendlyNPC] JSON Payload: " << payload << std::endl;

    globalApiClient.sendRequest(n8nWebhookUrl, payload, [this](const std::string& response) {
        this->onAPIResponse(response);
        });
}

void FriendlyNPC::onAPIResponse(const std::string& response) {
    std::cout << "[FriendlyNPC] ============ API RESPONSE DEBUG ============" << std::endl;
    std::cout << "[FriendlyNPC] Raw API Response:" << std::endl;
    std::cout << response << std::endl;
    std::cout << "[FriendlyNPC] Raw length: " << response.length() << std::endl;

    std::string cleanedResponse = response;

    // Look for {"output":"..."} or {"output": "..."} pattern
    size_t outputStart = response.find("\"output\"");
    if (outputStart != std::string::npos) {
        std::cout << "[FriendlyNPC] Found 'output' at position: " << outputStart << std::endl;

        // Find the colon after "output"
        size_t colonPos = response.find(":", outputStart);
        if (colonPos != std::string::npos) {
            std::cout << "[FriendlyNPC] Found ':' at position: " << colonPos << std::endl;

            // Find the opening quote after the colon
            size_t openQuotePos = response.find("\"", colonPos);
            if (openQuotePos != std::string::npos) {
                std::cout << "[FriendlyNPC] Found opening quote at position: " << openQuotePos << std::endl;

                // Start reading from after the opening quote
                size_t contentStart = openQuotePos + 1;
                std::cout << "[FriendlyNPC] Content starts at position: " << contentStart << std::endl;
                std::cout << "[FriendlyNPC] First 10 chars: '" << response.substr(contentStart, 10) << "'" << std::endl;

                // Find the closing quote, skipping escaped characters
                size_t currentPos = contentStart;
                bool foundEnd = false;

                while (currentPos < response.length()) {
                    if (response[currentPos] == '\\' && currentPos + 1 < response.length()) {
                        // Skip escaped character
                        currentPos += 2;
                        continue;
                    }
                    if (response[currentPos] == '"') {
                        // Found unescaped quote - this is the end
                        foundEnd = true;
                        std::cout << "[FriendlyNPC] Found closing quote at position: " << currentPos << std::endl;
                        break;
                    }
                    currentPos++;
                }

                if (foundEnd) {
                    cleanedResponse = response.substr(contentStart, currentPos - contentStart);
                    std::cout << "[FriendlyNPC] Extracted substring length: " << cleanedResponse.length() << std::endl;

                    // Unescape escaped quotes
                    size_t pos = 0;
                    while ((pos = cleanedResponse.find("\\\"", pos)) != std::string::npos) {
                        cleanedResponse.replace(pos, 2, "\"");
                        pos += 1;
                    }

                    // Unescape newlines
                    pos = 0;
                    while ((pos = cleanedResponse.find("\\n", pos)) != std::string::npos) {
                        cleanedResponse.replace(pos, 2, "\n");
                        pos += 1;
                    }

                    // Unescape backslashes
                    pos = 0;
                    while ((pos = cleanedResponse.find("\\\\", pos)) != std::string::npos) {
                        cleanedResponse.replace(pos, 2, "\\");
                        pos += 1;
                    }
                }
                else {
                    std::cerr << "[FriendlyNPC] ERROR: Could not find closing quote" << std::endl;
                }
            }
            else {
                std::cerr << "[FriendlyNPC] ERROR: Could not find opening quote after colon" << std::endl;
            }
        }
        else {
            std::cerr << "[FriendlyNPC] ERROR: Could not find colon after 'output'" << std::endl;
        }
    }
    else {
        std::cerr << "[FriendlyNPC] ERROR: Could not find 'output' field in response" << std::endl;
    }

    std::cout << "[FriendlyNPC] ============ FINAL OUTPUT ============" << std::endl;
    std::cout << "[FriendlyNPC] Cleaned Response: '" << cleanedResponse << "'" << std::endl;
    std::cout << "[FriendlyNPC] Cleaned length: " << cleanedResponse.length() << std::endl;
    std::cout << "[FriendlyNPC] First char ASCII: " << (int)cleanedResponse[0] << " ('" << cleanedResponse[0] << "')" << std::endl;
    std::cout << "[FriendlyNPC] =====================================" << std::endl;

    chatSystem.addMessage(characterName, cleanedResponse);
    chatSystem.setWaitingForResponse(false);
}

void FriendlyNPC::setScreenDimensions(int width, int height) {
    chatSystem.setScreenDimensions(width, height);
    std::cout << "[FriendlyNPC] Screen dimensions set to " << width << "x" << height << std::endl;
}