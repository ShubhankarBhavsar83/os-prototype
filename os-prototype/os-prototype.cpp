// os-prototype.cpp : Defines the entry point for the application.
// 51:18

#include "os-prototype.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <array>
#include <string>

#include "gameObject.h"

using namespace std;

struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	int sc_width, sc_height, logW, logH;

};

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
struct GameState {
	array<vector<GameObject>, 2> layers;
	int playerIndex;
	GameState() {
		playerIndex = 0;
	}

};

struct bankPath {

};

struct Resources {
	// todo - switch sprite (change to isometric sprite sheet)
	const int ANIM_PLAYER_IDLE = 0;
	vector<Animation> playerAnims;
	vector<SDL_Texture *> textures;
	SDL_Texture *idleTex;
	

	SDL_Texture* loadTexture(SDL_Renderer* renderer,const string &filepath) {
		// game assets
		SDL_Texture *tex = IMG_LoadTexture(renderer, filepath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;

		if (!tex) {
			std::cerr << "Failed to load texture: " << filepath << "\nError: " << SDL_GetError() << std::endl;
		}
	}

	void load(SDLState& state) {
		idleTex = loadTexture(state.renderer, "assets/player_assets/idle_right.png");
		playerAnims.resize(5);
		playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		
	}

	void unload() {
		for (SDL_Texture* tex : textures) {
			SDL_DestroyTexture(tex);
		}
	}
};

bool initialize(SDLState& state);
void cleanup(SDLState& state);

int main(int argc, char* argv[])
{
	cout << "App Start..." << endl;

	SDLState state{ 0 };

	state.sc_width = 1280;
	state.sc_height = 720;
	state.logW = 640;
	state.logH = 320;


	if (!initialize(state)) {
		return 1;
	}

	// game assets - loading 
	Resources res;
	res.load(state);


	// game data
	const bool* keys = SDL_GetKeyboardState(nullptr);
	float playerX = (float)(state.logW / 2);
	float playerY = (float)(state.logH / 2);
	cout << playerX << endl;
	cout << playerY << endl;
	bool flipHorizontal = false; // true =  left , false = right
	bool flipVertical = false; // true =  up , false = left

	// game loop
	bool runTopLoop = true;
	uint64_t previousTime = SDL_GetTicks();

	while (runTopLoop) {
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - previousTime) / 1000.0f;
		
		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT: {
				runTopLoop = false;
				break;
			}

			case SDL_EVENT_WINDOW_RESIZED:
				state.sc_width = event.window.data1;
				state.sc_height = event.window.data2;
			}
		}

		//movement // todo - iso movement and animaton calls
		float moveHorizontal = 0;
		float moveVertical = 0;


		if (keys[SDL_SCANCODE_A]) {
			moveHorizontal += -100.0f;
			flipHorizontal = true;
		}
		if (keys[SDL_SCANCODE_D]) {
			moveHorizontal += 100.0f;
			flipHorizontal = false;
		}
		if (keys[SDL_SCANCODE_W]) {
			moveVertical += -100.0f;
			flipVertical = true;
		}
		if (keys[SDL_SCANCODE_S]) {
			moveVertical += 100.0f;
			flipVertical = false;
		}

		playerX += moveHorizontal * deltaTime;
		playerY += moveVertical * deltaTime;

		// render tasks
		SDL_SetRenderDrawColor(state.renderer, 128, 128, 128, 255);
		SDL_RenderClear(state.renderer);

		 //sprite loading
		const float spriteSizeHorizontal = 80;
		const float spriteSizeVertical = 97;
		const float spriteSize = 32;
		

		SDL_FRect src{
			.x = 0,
			.y = 0,
			.w = spriteSizeHorizontal,
			.h = spriteSizeVertical

		};
		SDL_FRect dst{
			.x = playerX,
			.y = playerY,
			.w = spriteSize,
			.h = spriteSize
		};

		SDL_RenderTexture(state.renderer, res.idleTex, &src, &dst);

		SDL_RenderTextureRotated(state.renderer, res.idleTex, &src, &dst, 0, nullptr, (flipHorizontal) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);


		// buffer swap
		SDL_RenderPresent(state.renderer);
		previousTime = nowTime;
	}

	res.unload();
	cleanup(state);
	cout << "Shutting Down..." << endl;
	return 0;
}

bool initialize(SDLState& state) {

	bool initSuccess = true;


	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "SDL3 Initialization Failed.", 0);
		initSuccess = false;
	}


	//window
	state.window = SDL_CreateWindow("OS-PROT", state.sc_width, state.sc_height, SDL_WINDOW_RESIZABLE);
	if (!state.window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Window Initialization Failed.", state.window);
		cleanup(state);
		initSuccess = false;
	}

	// renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Renderer Initialization Failed.", state.window);
		cleanup(state);
		initSuccess = false;
	}

	// config presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	return initSuccess;
}

void cleanup(SDLState& state) {
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
	
}

void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime) {

}