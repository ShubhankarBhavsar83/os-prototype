#pragma once
#include <string>
#include <vector>
#include <functional>
#include <atomic>
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
    std::vector<float> messageHeights;
    std::string currentInput;
    bool active;
    std::atomic<bool> isRendering;

    SDL_FRect chatBoxRect;
    SDL_FRect inputBoxRect;
    int maxVisibleMessages;
    int scrollOffset;

    std::function<void(const std::string&)> onMessageSent;
    bool waitingForResponse;

    TTF_Font* font;
    SDL_Texture* fontTexture;

    // Screen dimensions
    int screenWidth;
    int screenHeight;

    void updateChatPosition();

public:
    ChatSystem();
    ~ChatSystem();
    ChatSystem(const ChatSystem&) = delete;
    ChatSystem& operator=(const ChatSystem&) = delete;

    void activate();
    void deactivate();
    bool isActive() const;

    void setScreenDimensions(int width, int height);
    void addMessage(const std::string& sender, const std::string& text);
    void handleTextInput(const std::string& text);
    void handleKeyPress(SDL_Keycode key);
    void sendCurrentMessage();
    void setMessageCallback(std::function<void(const std::string&)> callback);
    void setWaitingForResponse(bool waiting);
    void render(SDL_Renderer* renderer);
    void calculateMessageHeights();

    float renderMessage(SDL_Renderer* renderer, const ChatMessage& msg, float yOffset);
    void renderInputBox(SDL_Renderer* renderer);
};