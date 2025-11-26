//#pragma once
//#include <vector>
//#include <memory>
//#include "Entity.h"
//
//class EntityManager {
//private:
//    std::vector<std::unique_ptr<Entity>> entities;
//    std::vector<size_t> entitiesToRemove;
//
//public:
//    EntityManager();
//
//    void addEntity(std::unique_ptr<Entity> entity);
//    void removeEntity(size_t index);
//    void update(float deltaTime, class GameState& gs);
//    void render(SDL_Renderer* renderer, const SDL_FRect& viewport);
//    void checkCollisions();
//    void cleanup();
//
//
//    Entity* getEntityByID(int id);
//    std::vector<Entity*> getEntitiesInRange(const glm::vec2& pos, float range);
//    Player* getPlayer();
//};