#pragma once
#include <string>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>

struct APIRequest {
    std::string endpoint;
    std::string payload;
    std::function<void(const std::string&)> callback;
};

struct APIResponse {
    std::string data;
    bool success;
    std::string errorMessage;
};

class APIClient {
private:
    std::queue<APIRequest> requestQueue;
    std::queue<APIResponse> responseQueue;
    std::mutex requestMutex;
    std::mutex responseMutex;
    std::thread workerThread;
    bool running;

public:
    APIClient();
    ~APIClient();

    void start();
    void stop();

    void sendRequest(const std::string& endpoint,
        const std::string& payload,
        std::function<void(const std::string&)> callback);

    void processResponses();

private:
    void workerLoop();
    APIResponse makeHTTPRequest(const APIRequest& request);
    std::string buildJSONPayload(const std::string& message,
        const std::vector<ChatMessage>& history);
    std::string parseJSONResponse(const std::string& jsonData);
};