#include "ChatSystem.h"
#include <iostream>

ChatSystem::ChatSystem()
    : active(false), maxVisibleMessages(5), scrollOffset(0),
    waitingForResponse(false), font(nullptr), fontTexture(nullptr) {

    if (!TTF_WasInit() && TTF_Init() == -1) {
        std::cerr << "[ChatSystem] TTF_Init failed! Error: " << std::endl;
        return; // Stop here if init fails
    }

    chatBoxRect = { 0, 0, 300, 200 };
    inputBoxRect = { 0, 200, 300, 40 };

    font = TTF_OpenFont("assets/font/font.ttf", 16);
    if (!font) {
        std::cerr << "[ChatSystem] Failed to load font! Check assets/font.ttf" << std::endl;
    }
}

ChatSystem::~ChatSystem() {
    if (font) TTF_CloseFont(font);
    if (fontTexture) SDL_DestroyTexture(fontTexture);
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

void ChatSystem::setPosition(float x, float y) {
    // FIXED POSITION: Center the chat window on screen (popup style)
    // Assuming 640x320 logical screen size from Application.cpp
    float screenCenterX = 320.0f;  // Half of 640
    float screenCenterY = 160.0f;  // Half of 320

    // Center the chat box
    chatBoxRect.x = screenCenterX - (chatBoxRect.w / 2);
    chatBoxRect.y = screenCenterY - (chatBoxRect.h / 2) - 20; // Slightly above center

    inputBoxRect.x = chatBoxRect.x;
    inputBoxRect.y = chatBoxRect.y + chatBoxRect.h;

    std::cout << "[ChatSystem] Position set to CENTER (" << chatBoxRect.x << ", " << chatBoxRect.y << ")" << std::endl;
}

void ChatSystem::addMessage(const std::string& sender, const std::string& text) {
    ChatMessage msg;
    msg.sender = sender;
    msg.text = text;
    msg.timestamp = SDL_GetTicks();
    messages.push_back(msg);

    if (messages.size() > maxVisibleMessages) {
        scrollOffset = messages.size() - maxVisibleMessages;
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
    else if (key == SDLK_ESCAPE) {
        deactivate();
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
    if (!font) return;

    // Draw Chat Box Background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 230);
    SDL_RenderFillRect(renderer, &chatBoxRect);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderRect(renderer, &chatBoxRect);

    // Draw Messages
    int yStart = chatBoxRect.y + 10;
    int visibleCount = 0;

    size_t startIdx = (messages.size() > maxVisibleMessages) ? messages.size() - maxVisibleMessages : 0;

    for (size_t i = startIdx; i < messages.size(); ++i) {
        renderMessage(renderer, messages[i], yStart + (visibleCount * 30));
        visibleCount++;
    }

    // Draw Waiting Indicator
    if (waitingForResponse) {
        std::string waitText = "...";
        SDL_Surface* surf = TTF_RenderText_Blended(font, waitText.c_str(), 0, { 150, 150, 150, 255 });
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FRect dst = { chatBoxRect.x + 10, chatBoxRect.y + chatBoxRect.h - 30, (float)surf->w, (float)surf->h };
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
            SDL_DestroySurface(surf);
        }
    }

    renderInputBox(renderer);
}

float ChatSystem::renderMessage(SDL_Renderer* renderer, const ChatMessage& msg, float yOffset) {
    SDL_Color color = (msg.sender == "Player") ? SDL_Color{ 100, 200, 255, 255 } : SDL_Color{ 255, 200, 100, 255 };
    std::string display = msg.sender + ": " + msg.text;

    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, display.c_str(), display.length(), color, chatBoxRect.w - 20);

    float messageHeight = 0;
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FRect dst = { chatBoxRect.x + 10, yOffset, (float)surf->w, (float)surf->h };
        SDL_RenderTexture(renderer, tex, nullptr, &dst);
        messageHeight = (float)surf->h;
        SDL_DestroyTexture(tex);
        SDL_DestroySurface(surf);
    }

    return messageHeight;
}

void ChatSystem::renderInputBox(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &inputBoxRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &inputBoxRect);

    std::string textToShow = "> " + currentInput + (SDL_GetTicks() % 1000 < 500 ? "_" : "");
    SDL_Surface* surf = TTF_RenderText_Blended(font, textToShow.c_str(), 0, { 255, 255, 255, 255 });
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FRect dst = { inputBoxRect.x + 5, inputBoxRect.y + 5, (float)surf->w, (float)surf->h };
        SDL_RenderTexture(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
        SDL_DestroySurface(surf);
    }
}
