#include "ChatSystem.h"
#include <iostream>
#include <algorithm>

ChatSystem::ChatSystem()
    : active(false), maxVisibleMessages(5), scrollOffset(0),
    waitingForResponse(false), font(nullptr), fontTexture(nullptr),
    screenWidth(640), screenHeight(320), isRendering(false) {

    // TTF should already be initialized by Application, but check anyway
    if (!TTF_WasInit()) {
        std::cerr << "[ChatSystem] TTF not initialized! Attempting to initialize..." << std::endl;
        if (TTF_Init() == -1) {
            std::cerr << "[ChatSystem] TTF_Init failed! " << SDL_GetError() << std::endl;
            return;
        }
    }

    // Initialize with default positions
    updateChatPosition();

    // Load font - try multiple paths
    const char* fontPaths[] = {
        "assets/font/font.ttf",
        "assets/font.ttf",
        "../assets/font/font.ttf",
        "../assets/font.ttf"
    };

    for (const char* path : fontPaths) {
        font = TTF_OpenFont(path, 13);
        if (font) {
            std::cout << "[ChatSystem] Font loaded successfully from: " << path << std::endl;
            break;
        }
    }

    if (!font) {
        std::cerr << "[ChatSystem] CRITICAL: Failed to load font from all paths!" << std::endl;
        std::cerr << "[ChatSystem] TTF Error: " << SDL_GetError() << std::endl;
    }
}

ChatSystem::~ChatSystem() {
    std::cout << "[ChatSystem] Destructor called - waiting for render to finish..." << std::endl;

    // Wait for any active rendering to finish
    while (isRendering.load()) {
        SDL_Delay(1);
    }

    if (fontTexture) {
        SDL_DestroyTexture(fontTexture);
        fontTexture = nullptr;
        std::cout << "[ChatSystem] Font texture destroyed" << std::endl;
    }

    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
        std::cout << "[ChatSystem] Font closed" << std::endl;
    }

    std::cout << "[ChatSystem] Destructor complete" << std::endl;
}

void ChatSystem::activate() {
    active = true;
    SDL_StartTextInput(nullptr);
    std::cout << "[ChatSystem] Chat activated!" << std::endl;
}

void ChatSystem::deactivate() {
    active = false;
    SDL_StopTextInput(nullptr);
    std::cout << "[ChatSystem] Chat deactivated!" << std::endl;
}

bool ChatSystem::isActive() const {
    return active;
}

void ChatSystem::setScreenDimensions(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    updateChatPosition();
    std::cout << "[ChatSystem] Screen dimensions updated: " << width << "x" << height << std::endl;
}

void ChatSystem::updateChatPosition() {
    // Center the chat window on screen
    float chatWidth = 300.0f;
    float chatHeight = 200.0f;

    chatBoxRect.w = chatWidth;
    chatBoxRect.h = chatHeight;
    chatBoxRect.x = (screenWidth - chatWidth) / 2.0f;
    chatBoxRect.y = (screenHeight - chatHeight) / 2.0f - 20.0f;

    inputBoxRect.w = chatWidth;
    inputBoxRect.h = 40.0f;
    inputBoxRect.x = chatBoxRect.x;
    inputBoxRect.y = chatBoxRect.y + chatBoxRect.h;

    std::cout << "[ChatSystem] Chat positioned at (" << chatBoxRect.x << ", " << chatBoxRect.y << ")" << std::endl;
}

void ChatSystem::addMessage(const std::string& sender, const std::string& text) {
    ChatMessage msg;
    msg.sender = sender;
    msg.text = text;
    msg.timestamp = SDL_GetTicks();
    messages.push_back(msg);

    calculateMessageHeights();

    std::cout << "[ChatSystem] Message added. Total: " << messages.size() << std::endl;
}

void ChatSystem::calculateMessageHeights() {
    if (!font) {
        static bool warned = false;
        if (!warned) {
            std::cerr << "[ChatSystem] WARNING: Font is NULL in calculateMessageHeights()" << std::endl;
            warned = true;
        }
        // Set default heights without font
        messageHeights.clear();
        for (size_t i = 0; i < messages.size(); i++) {
            messageHeights.push_back(18.0f);
        }
        return;
    }

    messageHeights.clear();

    for (const auto& msg : messages) {
        std::string display = msg.sender + ": " + msg.text;

        // Validate string before rendering
        if (display.empty()) {
            messageHeights.push_back(18.0f);
            continue;
        }

        SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(
            font,
            display.c_str(),
            display.length(),
            { 255, 255, 255, 255 },
            (int)(chatBoxRect.w - 20)
        );

        if (surf) {
            messageHeights.push_back((float)surf->h + 5.0f);
            SDL_DestroySurface(surf);
        }
        else {
            messageHeights.push_back(18.0f);
            std::cerr << "[ChatSystem] Failed to render text surface for: " << display << std::endl;
        }
    }
}

void ChatSystem::handleTextInput(const std::string& text) {
    if (active && !waitingForResponse) {
        currentInput += text;
        std::cout << "[ChatSystem] Input: " << currentInput << std::endl;
    }
}

void ChatSystem::handleKeyPress(SDL_Keycode key) {
    if (!active) return;

    if (key == SDLK_RETURN) {
        sendCurrentMessage();
    }
    else if (key == SDLK_BACKSPACE && !currentInput.empty()) {
        currentInput.pop_back();
    }
}

void ChatSystem::sendCurrentMessage() {
    if (currentInput.empty() || waitingForResponse) return;

    addMessage("Player", currentInput);

    if (onMessageSent) {
        onMessageSent(currentInput);
    }

    currentInput = "";
    setWaitingForResponse(true);
}

void ChatSystem::setMessageCallback(std::function<void(const std::string&)> callback) {
    onMessageSent = callback;
}

void ChatSystem::setWaitingForResponse(bool waiting) {
    waitingForResponse = waiting;
}

void ChatSystem::render(SDL_Renderer* renderer) {
    if (!active) return;

    // Set rendering flag
    isRendering.store(true);

    if (!font) {
        std::cerr << "[ChatSystem] ERROR: Font is NULL in render()" << std::endl;
        isRendering.store(false);
        return;
    }

    if (messages.size() != messageHeights.size()) {
        calculateMessageHeights();
    }

    // Draw chat box background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 230);
    SDL_RenderFillRect(renderer, &chatBoxRect);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderRect(renderer, &chatBoxRect);

    // Set clip rect for messages
    SDL_Rect clipRect = {
        (int)chatBoxRect.x,
        (int)chatBoxRect.y,
        (int)chatBoxRect.w,
        (int)chatBoxRect.h
    };
    SDL_SetRenderClipRect(renderer, &clipRect);

    // Render messages from bottom up
    float currentY = chatBoxRect.y + chatBoxRect.h - 10;

    for (int i = (int)messages.size() - 1; i >= 0; i--) {
        if (i >= (int)messageHeights.size()) continue;

        float msgHeight = messageHeights[i];
        currentY -= msgHeight;

        renderMessage(renderer, messages[i], currentY);

        if (currentY < chatBoxRect.y - 50) break;
    }

    SDL_SetRenderClipRect(renderer, nullptr);

    // Draw waiting indicator at the BOTTOM of the chat box, just above input
    if (waitingForResponse && font) {
        std::string waitText = "NPC is typing...";

        SDL_Surface* surf = nullptr;
        try {
            surf = TTF_RenderText_Blended(font, waitText.c_str(), 0, { 200, 200, 100, 255 });
        }
        catch (...) {
            std::cerr << "[ChatSystem] Exception rendering waiting indicator" << std::endl;
        }

        if (surf) {
            // Draw background for indicator
            SDL_FRect bgRect = {
                chatBoxRect.x + 5,
                chatBoxRect.y + chatBoxRect.h - (float)surf->h - 10.0f,
                (float)surf->w + 10.0f,
                (float)surf->h + 5.0f
            };
            SDL_SetRenderDrawColor(renderer, 60, 60, 40, 200);
            SDL_RenderFillRect(renderer, &bgRect);

            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_FRect dst = {
                    chatBoxRect.x + 10,
                    chatBoxRect.y + chatBoxRect.h - (float)surf->h - 7.5f,
                    (float)surf->w,
                    (float)surf->h
                };
                SDL_RenderTexture(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_DestroySurface(surf);
        }
    }

    renderInputBox(renderer);

    // Clear rendering flag
    isRendering.store(false);
}

float ChatSystem::renderMessage(SDL_Renderer* renderer, const ChatMessage& msg, float yOffset) {
    if (!font) {
        std::cerr << "[ChatSystem] ERROR: Font is NULL in renderMessage()" << std::endl;
        return 0.0f;
    }

    SDL_Color color = (msg.sender == "Player") ?
        SDL_Color{ 100, 200, 255, 255 } :
        SDL_Color{ 255, 200, 100, 255 };

    std::string display = msg.sender + ": " + msg.text;

    // Validate string
    if (display.empty()) {
        return 0.0f;
    }

    SDL_Surface* surf = nullptr;

    try {
        surf = TTF_RenderText_Blended_Wrapped(
            font,
            display.c_str(),
            display.length(),
            color,
            (int)(chatBoxRect.w - 20)
        );
    }
    catch (...) {
        std::cerr << "[ChatSystem] Exception during text rendering" << std::endl;
        return 0.0f;
    }

    float messageHeight = 0;
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (tex) {
            SDL_FRect dst = {
                chatBoxRect.x + 10,
                yOffset,
                (float)surf->w,
                (float)surf->h
            };
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
            messageHeight = (float)surf->h;
            SDL_DestroyTexture(tex);
        }
        SDL_DestroySurface(surf);
    }
    else {
        std::cerr << "[ChatSystem] Failed to render text surface: " << display << std::endl;
    }

    return messageHeight;
}

void ChatSystem::renderInputBox(SDL_Renderer* renderer) {
    if (!font) {
        std::cerr << "[ChatSystem] ERROR: Font is NULL in renderInputBox()" << std::endl;
        return;
    }

    // Draw background
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &inputBoxRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &inputBoxRect);

    // Prepare text with cursor
    std::string textToShow = "> " + currentInput + (SDL_GetTicks() % 1000 < 500 ? "_" : "");

    if (textToShow.empty()) return;

    SDL_Surface* surf = nullptr;
    try {
        surf = TTF_RenderText_Blended(font, textToShow.c_str(), 0, { 255, 255, 255, 255 });
    }
    catch (...) {
        std::cerr << "[ChatSystem] Exception rendering input box text" << std::endl;
        return;
    }

    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (tex) {
            float textW = (float)surf->w;
            float textH = (float)surf->h;

            SDL_FRect visibleArea = {
                inputBoxRect.x + 5,
                inputBoxRect.y + 5,
                inputBoxRect.w - 10,
                inputBoxRect.h - 10
            };

            SDL_FRect dstRect;
            SDL_FRect srcRect;

            if (textW > visibleArea.w) {
                // Text too long, show right side
                srcRect.x = textW - visibleArea.w;
                srcRect.y = 0.0f;
                srcRect.w = visibleArea.w;
                srcRect.h = textH;
                dstRect = { visibleArea.x, visibleArea.y, visibleArea.w, textH };
            }
            else {
                // Text fits
                srcRect.x = 0.0f;
                srcRect.y = 0.0f;
                srcRect.w = textW;
                srcRect.h = textH;
                dstRect = { visibleArea.x, visibleArea.y, textW, textH };
            }

            SDL_Rect inputClip = {
                (int)inputBoxRect.x,
                (int)inputBoxRect.y,
                (int)inputBoxRect.w,
                (int)inputBoxRect.h
            };
            SDL_SetRenderClipRect(renderer, &inputClip);

            SDL_RenderTexture(renderer, tex, &srcRect, &dstRect);

            SDL_SetRenderClipRect(renderer, nullptr);

            SDL_DestroyTexture(tex);
        }
        SDL_DestroySurface(surf);
    }
    else {
        std::cerr << "[ChatSystem] Failed to render input text surface" << std::endl;
    }
}