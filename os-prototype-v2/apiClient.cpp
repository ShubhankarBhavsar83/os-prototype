#include "ApiClient.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

APIClient::APIClient() : running(false) {}

APIClient::~APIClient() { stop(); }

void APIClient::start() {
    running = true;
    workerThread = std::thread(&APIClient::workerLoop, this);
}

void APIClient::stop() {
    running = false;
    if (workerThread.joinable()) workerThread.join();
}

void APIClient::sendRequest(const std::string& endpoint, const std::string& payload, std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(requestMutex);
    requestQueue.push({ endpoint, payload, callback });
}

void APIClient::processResponses() {
    // Prototype: Responses are currently handled in callback threads
}

void APIClient::workerLoop() {
    while (running) {
        APIRequest req;
        bool hasRequest = false;

        {
            std::lock_guard<std::mutex> lock(requestMutex);
            if (!requestQueue.empty()) {
                req = requestQueue.front();
                requestQueue.pop();
                hasRequest = true;
            }
        }

        if (hasRequest) {
            APIResponse resp = makeHTTPRequest(req);
            if (req.callback) req.callback(resp.data);
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

APIResponse APIClient::makeHTTPRequest(const APIRequest& request) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    APIResponse response;

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, request.endpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            response.success = true;
            response.data = readBuffer;
        }
        else {
            response.success = false;
            response.errorMessage = curl_easy_strerror(res);
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return response;
}

std::string APIClient::buildJSONPayload(const std::string& message, const std::vector<ChatMessage>& history) {
    return "{ \"text\": \"" + message + "\" }";
}

std::string APIClient::parseJSONResponse(const std::string& jsonData) {
    return jsonData;
}