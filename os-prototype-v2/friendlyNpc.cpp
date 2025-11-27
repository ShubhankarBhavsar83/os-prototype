#include "FriendlyNpc.h"
#include "GameState.h"
#include "Player.h"
#include "ResourceManager.h" // Needed for loadTextures
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

    // Add initial greeting message (only once per instance)
    // chatSystem.addMessage("NPC", "Hello! Press E to chat."); // Optional: removed to prevent spam on re-open

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

    // 1. Extract pending messages safely
    std::vector<std::string> responsesToProcess;
    {
        std::lock_guard<std::mutex> lock(responseMutex);
        if (!pendingResponses.empty()) {
            responsesToProcess = pendingResponses;
            pendingResponses.clear();
        }
    }

    // 2. Process them on the Main Thread (Safe for SDL/TTF)
    for (const std::string& response : responsesToProcess) {
        std::string cleanedResponse = response;

        // --- JSON Parsing Logic ---
        size_t outputStart = response.find("\"output\"");
        if (outputStart != std::string::npos) {
            size_t colonPos = response.find(":", outputStart);
            if (colonPos != std::string::npos) {
                size_t openQuotePos = response.find("\"", colonPos);
                if (openQuotePos != std::string::npos) {
                    size_t contentStart = openQuotePos + 1;
                    size_t currentPos = contentStart;
                    bool foundEnd = false;
                    while (currentPos < response.length()) {
                        // Handle escaped characters
                        if (response[currentPos] == '\\' && currentPos + 1 < response.length()) {
                            currentPos += 2; continue;
                        }
                        if (response[currentPos] == '"') {
                            foundEnd = true; break;
                        }
                        currentPos++;
                    }
                    if (foundEnd) {
                        cleanedResponse = response.substr(contentStart, currentPos - contentStart);

                        // Basic unescaping
                        size_t pos = 0;
                        while ((pos = cleanedResponse.find("\\\"", pos)) != std::string::npos) {
                            cleanedResponse.replace(pos, 2, "\""); pos += 1;
                        }
                        pos = 0;
                        while ((pos = cleanedResponse.find("\\n", pos)) != std::string::npos) {
                            cleanedResponse.replace(pos, 2, "\n"); pos += 1;
                        }
                    }
                }
            }
        }

        // Update State
        isWaitingForResponse = false;
        chatSystem.setWaitingForResponse(false);
        lastResponse = cleanedResponse;

        // Add to Chat (SAFE now because we are in update())
        chatSystem.addMessage(characterName, cleanedResponse);

        std::cout << "[FriendlyNPC] Processed response on Main Thread." << std::endl;
    }
}

void FriendlyNPC::render(SDL_Renderer* renderer, const SDL_FRect& viewport) {
    // Green rectangle fallback if texture failed to load
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

    // Note: We removed the automatic "Hello" message here to prevent duplication
    // If the chat history is empty, you could add one here if desired.

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
    std::cout << "[FriendlyNPC] Received raw API response on background thread." << std::endl;

    // CRITICAL FIX: Do NOT parse here. Do NOT call chatSystem.addMessage here.
    // Just store the raw string safely. The 'update' function will handle the rest.
    std::lock_guard<std::mutex> lock(responseMutex);
    pendingResponses.push_back(response);
}

void FriendlyNPC::setScreenDimensions(int width, int height) {
    chatSystem.setScreenDimensions(width, height);
}

// FIX: Dynamic texture loading based on setNPCImage
void FriendlyNPC::loadTextures(ResourceManager& rm) {
    std::string key = getNPCImage();

    // Default fallback if no key was set
    if (key.empty()) {
        key = "npc_elara";
    }

    texture = rm.getTexture(key);

    if (!texture) {
        std::cout << "[FriendlyNPC] Warning: Could not find texture for key: " << key << ". Trying fallback." << std::endl;
        // Try to load it if not found (simple check)
        rm.loadTexture(key, "assets/npcs/" + key + ".png");
        texture = rm.getTexture(key);

        // Final fallback to enemy placeholder
        if (!texture) {
            texture = rm.getTexture("enemy_placeholder");
        }
    }
}