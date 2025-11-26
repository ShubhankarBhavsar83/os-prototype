# CMakeList.txt : CMake project for os-prototype-v2, include source and define
# project specific logic here.
#

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(glm REQUIRED)

# Add source to this project's executable.
add_executable (os-prototype-v2 
"os-prototype-v2.cpp" 
    "os-prototype-v2.h" 
    "animation.h" 
    "timer.h" 
    "entity.h" 
    "collider.h" 
    "player.h" 
    "player.cpp"
    "npc.h"
    "npc.cpp"
    "friendlyNpc.h" 
    "friendlyNpc.cpp"
    "enemyNpc.h"
    "enemyNpc.cpp"         
    "projectile.h" 
    "projectile.cpp"       
    "furniture.h" 
    "furniture.cpp"
    "entityManager.h" 
    "levelLoader.h" 
    "levelLoader.cpp"
    "resourceManager.h" 
    "gameState.h" 
    "gameState.cpp"
    "chatSystem.h" 
    "apiClient.h" 
    "application.h"
    "application.cpp"
    "coordinateSystem.h"
    "portal.h"
    "uiManager.h"          
    "uiManager.cpp"        
    "movable.h"         
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET os-prototype-v2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(os-prototype-v2 SDL3::SDL3 SDL3_image::SDL3_image glm::glm)