// os - prototype.cpp : Defines the entry point for the application.
//

#include "os-prototype.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>


using namespace std;

struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	int sc_width, sc_height, logW, logH;

};

bool initialize(SDLState & state);
void cleanup(SDLState & state);

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

	// game assets
	SDL_Texture* idleTex = IMG_LoadTexture(state.renderer, "assets/player_assets/idle_down_debug.png");
	SDL_Texture* runRightTex = IMG_LoadTexture(state.renderer, "assets/player_assets/run_right_debug.png");
	SDL_SetTextureScaleMode(idleTex, SDL_SCALEMODE_NEAREST);

	// game data
	const bool* keys = SDL_GetKeyboardState(nullptr);
	float playerX = (float)(state.logW / 2);
	float playerY = (float)(state.logH / 2);
	bool verticalOrientation = false; // false = down, true = up
	bool horizontalOrientation = false; // false = right, true = left
	

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

		//movement
		float moveHorizontal = 0;
		float moveVertical = 0;
		if (keys[SDL_SCANCODE_A]) {
			moveHorizontal += -100.0f;
			horizontalOrientation = true;
		}
		if (keys[SDL_SCANCODE_D]) {
			moveHorizontal += 100.0f;
			horizontalOrientation = false;
		}
		if (keys[SDL_SCANCODE_W]) {
			moveVertical += -100.0f;
			verticalOrientation = true;
		}
		if (keys[SDL_SCANCODE_S]) {
			moveVertical += 100.0f;
			verticalOrientation = false;
		}

		playerX += moveHorizontal * deltaTime;
		playerY += moveVertical * deltaTime;


		// render tasks
		SDL_SetRenderDrawColor(state.renderer, 128, 128, 128, 255);
		SDL_RenderClear(state.renderer);

		// sprite loading
		const float spriteSize = 32;

		SDL_FRect src{
			.x = 0,
			.y = 0,
			.w = 96,
			.h = 80
		};
		SDL_FRect dst{
			.x = playerX,
			.y = playerY,
			.w = 96 * 0.50,
			.h = 80 * 0.50
		};

		//SDL_RenderTexture(state.renderer, idleTex, &src, &dst);
		SDL_RenderTextureRotated(state.renderer, runRightTex, &src, &dst, 0, nullptr, (horizontalOrientation) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

		// buffer swap 
		SDL_RenderPresent(state.renderer);
		previousTime = nowTime;
	}

	SDL_DestroyTexture(idleTex);
	cleanup(state);
	cout << "Shutting Down..." << endl;
	return 0;
}

bool initialize(SDLState & state) {

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

void cleanup(SDLState & state) {
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}