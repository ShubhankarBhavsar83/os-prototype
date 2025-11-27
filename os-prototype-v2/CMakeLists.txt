# CMakeList.txt : CMake project for os-prototype-v2, include source and define
# project specific logic here.
#

cmake_minimum_required(VERSION 3.12)
project(os-prototype-v2)

# --- FIX START: TELL CMAKE WHERE YOUR LIBRARIES ARE ---
# We append your library paths so find_package can locate Config.cmake files
list(APPEND CMAKE_PREFIX_PATH "D:/codingStudy/SDL3-devel-3.2.20-VC/SDL3-3.2.20")
list(APPEND CMAKE_PREFIX_PATH "D:/codingStudy/SDL3_image-devel-3.2.4-VC/SDL3_image-3.2.4")
list(APPEND CMAKE_PREFIX_PATH "D:/codingStudy/SDL3_ttf-devel-3.2.2-VC/SDL3_ttf-3.2.2")
# NOTE: You also need to add the path to GLM here if it is not installed globally!
# list(APPEND CMAKE_PREFIX_PATH "D:/codingStudy/path/to/glm") 
# ------------------------------------------------------

include(FetchContent)

FetchContent_Declare(curl
	URL https://curl.se/download/curl-8.5.0.tar.gz
	DOWNLOAD_EXTRACT_TIMESTAMP true
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(curl)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)
find_package(curl)


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
    "chatSystem.cpp"
    "apiClient.h" 
    "apiClient.cpp"
    "application.h"
    "application.cpp"
    "coordinateSystem.h"
    "portal.h"
    "uiManager.h"          
    "uiManager.cpp"        
    "movable.h" 
    
)

# Assuming your executable target is named 'os-prototype-v2'
add_custom_command(TARGET os-prototype-v2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:libcurl>
    $<TARGET_FILE_DIR:os-prototype-v2>
    COMMENT "Copying libcurl DLL to executable directory"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET os-prototype-v2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(os-prototype-v2 SDL3::SDL3 SDL3_image::SDL3_image glm::glm SDL3_ttf::SDL3_ttf CURL::libcurl)