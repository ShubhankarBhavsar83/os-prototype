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
	const bool* keys;
	SDLState() : keys(SDL_GetKeyboardState(nullptr)) {

	}

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
		playerAnimations[ANIM_PLAYER_IDLE] = Animation(8, 1.0);

		texIdle = loadTexture(state.renderer, "assets/player_assets/idle_down.png");
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
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);

int main(int argc, char* argv[])
{
	cout << "App Start..." << endl;

	SDLState state;

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
	//player.acceleration = glm::vec2(0, 100);
	player.maxSpeedX = 100;
	player.maxSpeedY = 100;


	gs.layers[LAYER_IDX_CHARACTERS].push_back(player);

 
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

		// update game objects
		for (auto& layer : gs.layers) {
			for (GameObject& obj : layer) {

				update(state, gs, res, obj, deltaTime);
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
				drawObject(state, gs, obj, deltaTime);
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

}

void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {

	if (obj.type == ObjectType::player) {

		float accX = 0.0;
		float accY = 0.0;

		struct MoveTest {
			float X = 0.0;
			float Y = 0.0;
		};
		
		MoveTest mt;

		////movement 
		float HorizontalDirection = 0;
		float VerticalDirection = 0;
		if (state.keys[SDL_SCANCODE_A]) {
			//HorizontalDirection += 1;
			accX += -100;
		}
		if (state.keys[SDL_SCANCODE_D]) {
			//HorizontalDirection += 1;
			accX += 100;
		}
		if (state.keys[SDL_SCANCODE_W]) {
			//VerticalDirection += 1;
			accY += -100;
		}
		if (state.keys[SDL_SCANCODE_S]) {

			//VerticalDirection += 1;
			accY += 100;

		}
		if ((mt.X != 0) || (mt.Y != 0)) {
			obj.directionHorizontal = mt.X;
			obj.directionVertical = mt.Y;
		}

		switch (obj.data.player.state) {
	
			case PlayerState::idle: {
				if ((mt.X != 0) || (mt.Y != 0)) {
					obj.data.player.state = PlayerState::running;
				}
				break;
			}
			case PlayerState::running: {
				if ((mt.X == 0) && (mt.Y == 0)) {
					obj.data.player.state = PlayerState::idle;
				}
				break;
			}
		}


		obj.acceleration = glm::vec2(accX, accY);



		// add acceleration to velocity 
		obj.velocity += obj.acceleration * deltaTime;
		obj.velocity += obj.acceleration * deltaTime;


		//float veloX = obj.velocity + (VerticalDirection * obj.acceleration * deltaTime);

		//if (std::abs(obj.velocity.x) > obj.maxSpeedX ) {
		//	//obj.acceleration.x -= accX;
		//	obj.velocity.x = ((std::abs(obj.acceleration.x)) / obj.acceleration.x)* obj.maxSpeedX;


		//}
		//else if (std::abs(obj.velocity.y) > obj.maxSpeedY) {
		//	//obj.acceleration.y -= accY;
		//	obj.velocity.y = (std::abs(obj.acceleration.y) / obj.acceleration.y) * obj.maxSpeedY;


		//}


		if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
			obj.velocity.x = (obj.velocity.x > 0 ? 1 : -1) * obj.maxSpeedX;
		}
		if (std::abs(obj.velocity.y) > obj.maxSpeedY) {
			obj.velocity.y = (obj.velocity.y > 0 ? 1 : -1) * obj.maxSpeedY;
		}

		//cout << "------**-----" << endl;
		//cout << accX << "- accel x" << endl;
		//cout << accY << "- accel y" << endl;
		//cout << obj.velocity.x << "- vel x" << endl;
		//cout << obj.velocity.y << "- vel y" << endl;
		//cout << obj.acceleration.x << "obj accel x" << endl;
		//cout << obj.acceleration.y << "obj accel y" << endl;
		//cout << "-------------" << endl;


		// add velocity to position
		obj.position += obj.velocity * deltaTime;

	}

}