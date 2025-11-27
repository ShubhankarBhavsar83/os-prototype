#include "FriendlyNpc.h"
#include "GameState.h"
#include "Player.h"
#include <iostream>

// Static member for tracking active chat
FriendlyNPC* FriendlyNPC::activeChatNPC = nullptr;

// Global API client
static APIClient globalApiClient;

FriendlyNPC::FriendlyNPC()
    : NPC(NPCType::FRIENDLY),
    isWaitingForResponse(false),
    characterName("elara"),
    playerID("001") {

    this->type = EntityType::FRIENDLY_NPC;
    this->solid = true;
    this->spriteWidth = 32.0f;
    this->spriteHeight = 32.0f;
    this->scale = 1.0f;
    this->friction = 900.0f;

    // Initialize global API client once
    static bool apiStarted = false;
    if (!apiStarted) {
        globalApiClient.start();
        apiStarted = true;
    }

    // Set up the chat system with callback
    chatSystem.setMessageCallback([this](const std::string& message) {
        this->sendToAPI(message);
        });

    // Add initial greeting message
    chatSystem.addMessage("NPC", "Hello! Press E to chat.");

    std::cout << "[FriendlyNPC] Created with character: " << characterName << std::endl;
}

FriendlyNPC::~FriendlyNPC() {
    std::cout << "[FriendlyNPC] Destructor called" << std::endl;

    // If this NPC was the active chat, clear it
    if (activeChatNPC == this) {
        std::cout << "[FriendlyNPC] Clearing active chat NPC" << std::endl;
        activeChatNPC = nullptr;
    }
}

void FriendlyNPC::update(float deltaTime, GameState& gs) {
    // Physics
    applyMovement(deltaTime, maxSpeedX, maxSpeedY);
}

void FriendlyNPC::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    // Green rectangle fallback
    if (!texture) {
        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
        SDL_FRect dst = {
            position.x - viewport.x,
            position.y - viewport.y,
            spriteWidth * scale,
            spriteHeight * scale
        };
        SDL_RenderFillRect(renderer, &dst);
    }
    else {
        SDL_FRect dst = {
            position.x - viewport.x,
            position.y - viewport.y,
            spriteWidth * scale,
            spriteHeight * scale
        };
        SDL_RenderTexture(renderer, texture, nullptr, &dst);
    }

    // Render chat UI if this NPC's chat is active
    if (chatSystem.isActive()) {
        chatSystem.render(renderer);
    }
}

void FriendlyNPC::onPlayerInteract(Player* player) {
    if (!chatSystem.isActive()) {
        startChat();
    }
    else {
        endChat();
    }
}

void FriendlyNPC::startChat() {
    std::cout << "[FriendlyNPC] Starting chat..." << std::endl;

    // Close any other active chat
    if (activeChatNPC && activeChatNPC != this) {
        std::cout << "[FriendlyNPC] Closing previous NPC's chat" << std::endl;
        activeChatNPC->endChat();
    }

    activeChatNPC = this;
    chatSystem.activate();

    std::cout << "[FriendlyNPC] Chat started. Active NPC: " << this << std::endl;
}

void FriendlyNPC::endChat() {
    std::cout << "[FriendlyNPC] Ending chat..." << std::endl;

    chatSystem.deactivate();

    // Only clear if we're still the active NPC
    if (activeChatNPC == this) {
        activeChatNPC = nullptr;
    }

    std::cout << "[FriendlyNPC] Chat ended." << std::endl;
}

void FriendlyNPC::handleTextInput(const std::string& text) {
    chatSystem.handleTextInput(text);
}

void FriendlyNPC::handleKeyDown(SDL_Keycode key) {
    chatSystem.handleKeyPress(key);
}

void FriendlyNPC::sendToAPI(const std::string& message) {
    if (isWaitingForResponse) {
        std::cout << "[FriendlyNPC] Already waiting for response, ignoring new message" << std::endl;
        return;
    }

    std::cout << "[FriendlyNPC] Sending message to API: " << message << std::endl;

    // Escape quotes in the message
    std::string escapedMessage = message;
    size_t pos = 0;
    while ((pos = escapedMessage.find("\"", pos)) != std::string::npos) {
        escapedMessage.replace(pos, 1, "\\\"");
        pos += 2;
    }

    // Build JSON payload with proper formatting
    std::string payload = "{ \"characterName\":\"" + characterName +
        "\", \"context\":\"" + escapedMessage +
        "\", \"playerID\":\"" + playerID + "\" }";

    std::cout << "[FriendlyNPC] JSON Payload: " << payload << std::endl;

    isWaitingForResponse = true;
    chatSystem.setWaitingForResponse(true);

    globalApiClient.sendRequest(
        n8nWebhookUrl,
        payload,
        [this](const std::string& response) {
            this->onAPIResponse(response);
        }
    );
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
    if (!cleanedResponse.empty()) {
        std::cout << "[FriendlyNPC] First char ASCII: " << (int)cleanedResponse[0] << " ('" << cleanedResponse[0] << "')" << std::endl;
    }
    std::cout << "[FriendlyNPC] =====================================" << std::endl;

    isWaitingForResponse = false;
    chatSystem.setWaitingForResponse(false);
    chatSystem.addMessage(characterName, cleanedResponse);

    lastResponse = cleanedResponse;
}

void FriendlyNPC::setScreenDimensions(int width, int height) {
    chatSystem.setScreenDimensions(width, height);
    std::cout << "[FriendlyNPC] Screen dimensions set to " << width << "x" << height << std::endl;
}