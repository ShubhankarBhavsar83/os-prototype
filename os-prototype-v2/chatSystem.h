#pragma once
#include <string>
#include <vector>
#include <functional>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

struct ChatMessage {
    std::string sender;
    std::string text;
    uint64_t timestamp;
};

class ChatSystem {
private:
    std::vector<ChatMessage> messages;
    std::string currentInput;
    bool active;

    // UI properties
    SDL_FRect chatBoxRect;
    SDL_FRect inputBoxRect;
    int maxVisibleMessages;
    int scrollOffset;

    // API callback
    std::function<void(const std::string&)> onMessageSent;
    bool waitingForResponse;

    // Text rendering
    TTF_Font* font;
    SDL_Texture* fontTexture;

public:
    ChatSystem();
    ~ChatSystem();

    void activate();
    void deactivate();
    bool isActive() const;

    void setPosition(float x, float y);
    void addMessage(const std::string& sender, const std::string& text);
    void handleTextInput(const std::string& text);
    void handleKeyPress(SDL_Keycode key);
    void sendCurrentMessage();
    void setMessageCallback(std::function<void(const std::string&)> callback);
    void setWaitingForResponse(bool waiting);
    void render(SDL_Renderer* renderer);

    // Helpers - updated signature to return height
    float renderMessage(SDL_Renderer* renderer, const ChatMessage& msg, float yOffset);
    void renderInputBox(SDL_Renderer* renderer);
};
