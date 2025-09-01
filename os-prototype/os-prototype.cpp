// os - prototype.cpp : Defines the entry point for the application.
//

#include "os-prototype.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>

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
		playerIndex = 0; // todo - update when loading maps
	}
};

struct Resources {
	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUN_RIGHT = 1;
	const int ANIM_PLAYER_RUN_LEFT = 2;
	const int ANIM_PLAYER_RUN_UP = 3;
	const int ANIM_PLAYER_RUN_DOWN = 4;

	vector<Animation> playerAnimations;

	vector <SDL_Texture*> textures;
	SDL_Texture* texIdle;
	SDL_Texture* texRunRight;
	SDL_Texture* texRunLeft;
	SDL_Texture* texRunUp;
	SDL_Texture* texRunDown;


	SDL_Texture* loadTexture(SDL_Renderer *renderer, const string &filepath) {

		SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	void load(SDLState &state) {
		playerAnimations.resize(5);
		playerAnimations[ANIM_PLAYER_IDLE] = Animation(8, 1.6);

		texIdle = loadTexture(state.renderer, "assets/player_assets/idle_down_debug.png");
		texRunRight = IMG_LoadTexture(state.renderer, "assets/player_assets/debug/run_right_debug.png");
		texRunLeft = IMG_LoadTexture(state.renderer, "assets/player_assets/debug/run_left_debug.png");
		texRunUp = IMG_LoadTexture(state.renderer, "assets/player_assets/debug/run_up_debug.png");
		texRunDown = IMG_LoadTexture(state.renderer, "assets/player_assets/debug/run_down_debug.png");

	}

	void unload() {

		for (SDL_Texture* tex : textures) {
			SDL_DestroyTexture(tex);
		}
	}

};

bool initialize(SDLState & state);
void cleanup(SDLState & state);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float &deltaTime);

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

	Resources res;
	res.load(state);


	// game data
	GameState gs;

	// player data
	GameObject player;
	player.type = ObjectType::player;
	player.texture = res.texIdle;
	player.animations = res.playerAnimations;
	player.currentAnimation = res.ANIM_PLAYER_IDLE;
	gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
	float srcx = 0.0f;

	char orientation = '3'; // 1 = up, 2 = right, 3 = down, 4 = left.
	//bool moving = false; // animation switching from move to idle --     ///  -- todo - manage movement animation

	const bool* keys = SDL_GetKeyboardState(nullptr);
	//float playerX = (float)(state.logW / 2);
	//float playerY = (float)(state.logH / 2);
	//bool verticalOrientation = false; // false = down, true = up
	//bool horizontalOrientation = false; // false = right, true = left

 
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

		////movement 
		////  todo - update to switch statement
		//float moveHorizontal = 0;
		//float moveVertical = 0;
		//if (keys[SDL_SCANCODE_A]) {
		//	moveHorizontal += -100.0f;
		//	//horizontalOrientation = true;
		//	orientation = '4';
		//}
		//if (keys[SDL_SCANCODE_D]) {
		//	moveHorizontal += 100.0f;
		//	//horizontalOrientation = false;
		//	orientation = '2';
		//}
		//if (keys[SDL_SCANCODE_W]) {
		//	moveVertical += -100.0f;
		//	//verticalOrientation = true;
		//	orientation = '1';
		//}
		//if (keys[SDL_SCANCODE_S]) {
		//	moveVertical += 100.0f;
		//	//verticalOrientation = false;
		//	orientation = '3';
		//}

		//playerX += moveHorizontal * deltaTime;
		//playerY += moveVertical * deltaTime;


		// update game objects
		for (auto& layer : gs.layers) {
			for (GameObject& obj : layer) {
				if (obj.currentAnimation != -1) {
					obj.animations[obj.currentAnimation].step(deltaTime);
				}
			}
		}


		// render tasks
		SDL_SetRenderDrawColor(state.renderer, 128, 128, 128, 255);
		SDL_RenderClear(state.renderer);


		// draw layer wise objects
		for (auto& layer : gs.layers) {
			for (GameObject &obj : layer) {
				drawObject(state, gs, obj, deltaTime );
			}
		}

		// buffer swap 
		SDL_RenderPresent(state.renderer);
		previousTime = nowTime;
	}

	//SDL_DestroyTexture(idleTex);
	res.unload();
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

void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float &deltaTime) {


	const float spriteWidth = 96.0f;
	const float spriteHeight = 80.0f;

	

	float srcX = obj.currentAnimation != -1 ? obj.animations[obj.currentAnimation].currentFrame() * spriteWidth : 0.0f;


	SDL_FRect src{
		.x = srcX,
		.y = 0,
		.w = spriteWidth,
		.h = spriteHeight
	};
	SDL_FRect dst{
		.x = obj.position.x,
		.y = obj.position.y,
		//.x = (float)state.logW / 2,
		//.y = (float)state.logH / 2,
		.w = spriteWidth * 0.50,
		.h = spriteHeight * 0.50
	};

	SDL_RenderTexture(state.renderer, obj.texture, &src, &dst);



	//switch (orientation) {
	//	case '1': {
	//		//SDL_RenderTexture(state.renderer, res.texRunUp, &src, &dst);
	//		SDL_RenderTexture(state.renderer, obj.texture, &src, &dst);

	//		break;
	//	}
	//	case '2': {
	//		//SDL_RenderTexture(state.renderer, res.texRunRight, &src, &dst);
	//		SDL_RenderTexture(state.renderer, obj.texture, &src, &dst);
	//		break;
	//	}
	//	case '3': {
	//		//SDL_RenderTexture(state.renderer, res.texRunDown, &src, &dst);
	//		SDL_RenderTexture(state.renderer, obj.texture, &src, &dst);
	//		break;
	//	}
	//	case '4': {
	//		//SDL_RenderTexture(state.renderer, res.texRunLeft, &src, &dst);
	//		SDL_RenderTexture(state.renderer, obj.texture, &src, &dst);
	//		break;
	//	}
	//}

	//SDL_RenderTextureRotated(state.renderer, runRightTex, &src, &dst, 0, nullptr, (horizontalOrientation) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

}