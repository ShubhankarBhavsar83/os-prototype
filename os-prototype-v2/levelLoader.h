#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>


struct LevelData {
    int width;
    int height;
    std::vector<std::vector<int>> tileLayer;
    std::vector<std::vector<int>> furnitureLayer;
    std::vector<std::vector<int>> npcLayer;
    glm::vec2 playerSpawn;
};

class LevelLoader {
private:
    std::string currentLevelPath;

public:
    LevelLoader();

    LevelData loadFromCSV(const std::string& filepath);
    void parseCSVLine(const std::string& line, std::vector<int>& output);
    void populateGameState(const LevelData& data, class GameState& gs, class Resources& res);

private:
    int getTileIDFromCSV(int csvValue);
    EntityType getEntityTypeFromCSV(int csvValue);
};