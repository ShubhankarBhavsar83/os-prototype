// os - prototype.cpp : Defines the entry point for the application.
// 1:05:30

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
const size_t LAYER_IDX_FURNITURE = 1;
const size_t LAYER_IDX_CHARACTERS = 2;
const int MAP_ROWS = 25;
const int MAP_COLS = 25;
const int TILE_SIZE = 32;

struct GameState {
	array<vector<GameObject>, 3> layers;
	int playerIndex;

	GameState() {
		playerIndex = 0; // todo - update when loading maps
	}
};

struct Resources {
	const int ANIM_PLAYER_IDLE_RIGHT = 0;
	const int ANIM_PLAYER_IDLE_LEFT = 1;
	const int ANIM_PLAYER_IDLE_UP = 2;
	const int ANIM_PLAYER_IDLE_DOWN = 3;

	const int ANIM_PLAYER_RUN_RIGHT = 4;
	const int ANIM_PLAYER_RUN_LEFT = 5;
	const int ANIM_PLAYER_RUN_UP = 6;
	const int ANIM_PLAYER_RUN_DOWN = 7;

	vector<Animation> playerAnimations;

	vector <SDL_Texture*> textures;
	SDL_Texture *texRunRight, *texRunLeft, *texRunUp, *texRunDown,
				*texIdleRight, *texIdleLeft, *texIdleUp, *texIdleDown,
				*texDirt, *texGrass, *texDirtPillar;

	SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& filepath) {

		SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	void load(SDLState& state) {
		playerAnimations.resize(8);
		playerAnimations[ANIM_PLAYER_IDLE_RIGHT] = Animation(8, 0.7);
		playerAnimations[ANIM_PLAYER_IDLE_LEFT] = Animation(8, 0.7);
		playerAnimations[ANIM_PLAYER_IDLE_UP] = Animation(8, 0.7);
		playerAnimations[ANIM_PLAYER_IDLE_DOWN] = Animation(8, 0.7);

		playerAnimations[ANIM_PLAYER_RUN_RIGHT] = Animation(8, 0.7);
		playerAnimations[ANIM_PLAYER_RUN_LEFT] = Animation(8, 0.7);
		playerAnimations[ANIM_PLAYER_RUN_UP] = Animation(8, 0.7);
		playerAnimations[ANIM_PLAYER_RUN_DOWN] = Animation(8, 0.7);


		texIdleRight = loadTexture(state.renderer, "assets/player_assets/idle_right.png");
		texIdleLeft = loadTexture(state.renderer, "assets/player_assets/idle_left.png");
		texIdleUp = loadTexture(state.renderer, "assets/player_assets/idle_up.png");
		texIdleDown = loadTexture(state.renderer, "assets/player_assets/idle_down.png");

		texRunRight = loadTexture(state.renderer, "assets/player_assets/run_right.png");
		texRunLeft = loadTexture(state.renderer, "assets/player_assets/run_left.png");
		texRunUp = loadTexture(state.renderer, "assets/player_assets/run_up.png");
		texRunDown = loadTexture(state.renderer, "assets/player_assets/run_down.png");

		texDirt = loadTexture(state.renderer, "assets/map_assets/tile_003.png");
		texGrass = loadTexture(state.renderer, "assets/map_assets/tile_040.png");
		texDirtPillar = loadTexture(state.renderer, "assets/map_assets/tile_059.png");


	}

	void unload() {

		for (SDL_Texture* tex : textures) {
			SDL_DestroyTexture(tex);
		}
	}

};


//fuctions
bool initialize(SDLState& state);
void cleanup(SDLState& state);
inline glm::vec2 orthoToIso(int col, int row, int tileSize, const SDLState& state);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float& deltaTime);
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);
auto createObject(int r, int c, SDL_Texture* tex, ObjectType type, const SDLState& state);
auto drawAll(short map[MAP_ROWS][MAP_COLS], int r, int c, const SDLState& state,GameState& gs, const Resources& res);
void createTiles(const SDLState& state, GameState& gs, const Resources& res);

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
	createTiles(state, gs, res);

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
			for (GameObject& obj : layer) {
				drawObject(state, gs, obj, deltaTime);
			}
		}

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

inline glm::vec2 orthoToIso(int col, int row, int tileSize ,const SDLState& state) {
	int tileWidth = tileSize;
	int tileHeight = tileSize / 2;

	float isoX = (col - row) * (tileWidth / 2.0f);
	float isoY = (col + row) * (tileHeight / 2.0f);

	float offsetX = state.logW / 2;
	float offsetY = 0;

	return glm::vec2(isoX + offsetX, isoY + offsetY);
}

// iso to ortho method ----
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float& deltaTime) {


	const float spriteWidth = obj.sprite_width;
	const float spriteHeight = obj.sprite_height;



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
		.w = spriteWidth * obj.scale,
		.h = spriteHeight * obj.scale
	};

	SDL_RenderTexture(state.renderer, obj.texture, &src, &dst);

}

void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {

	if (obj.type == ObjectType::player) {

		float accX = 0.0;
		float accY = 0.0;

		struct MoveDirectionSet {
			float X = 0.0;
			float Y = 0.0;
		};

		MoveDirectionSet mds{ 0, 0 };

		////movement 
		float HorizontalDirection = 0;
		float VerticalDirection = 0;
		if (state.keys[SDL_SCANCODE_A]) {
			mds.X += -1;
			accX += -500;
		}
		if (state.keys[SDL_SCANCODE_D]) {
			mds.X += 1;
			accX += 500;
		}
		if (state.keys[SDL_SCANCODE_W]) {
			mds.Y += -1;
			accY += -500;
		}
		if (state.keys[SDL_SCANCODE_S]) {
			mds.Y += 1;
			accY += 500;
		}


		obj.acceleration = glm::vec2(accX, accY);


		switch (obj.data.player.state) {

		case PlayerState::idle: {
			if ((mds.X != 0) || (mds.Y != 0)) {
				obj.data.player.state = PlayerState::running;
			}
			else {
				// deceleration for X movement
				if (obj.velocity.x != 0) {
					const float factor = obj.velocity.x > 0 ? accX = 650 : accX = -650;
					float amount = factor * accX /*base acceleration value*/ * deltaTime;
					if (std::abs(obj.velocity.x) < std::abs(amount)) {
						obj.velocity.x = 0;
					}
					else {
						obj.velocity.x += amount;
					}
				}
				// deceleration for Y movement
				if (obj.velocity.y != 0) {
					const float factor = obj.velocity.y > 0 ? accY = 650 : accY = -650;
					float amount = factor * accY /*base acceleration value*/ * deltaTime;
					if (std::abs(obj.velocity.y) < std::abs(amount)) {
						obj.velocity.y = 0;

					}
					else {

						obj.velocity.y += amount;
					}
				}
			}
			break;
		}
		case PlayerState::running: {
			if ((mds.X == 0) && (mds.Y == 0)) {
				obj.data.player.state = PlayerState::idle;

				if (obj.directionHorizontal > 0) {
					obj.texture = res.texIdleRight;
					obj.currentAnimation = res.ANIM_PLAYER_IDLE_RIGHT;
				}
				else if (obj.directionHorizontal < 0) {
					obj.texture = res.texIdleLeft;
					obj.currentAnimation = res.ANIM_PLAYER_IDLE_LEFT;
				}
				else if (obj.directionVertical < 0) {
					obj.texture = res.texIdleUp;
					obj.currentAnimation = res.ANIM_PLAYER_IDLE_UP;
				}
				else if (obj.directionVertical > 0) {
					obj.texture = res.texIdleDown;
					obj.currentAnimation = res.ANIM_PLAYER_IDLE_DOWN;
				}
			}
			else {

				obj.directionHorizontal = mds.X;
				obj.directionVertical = mds.Y;

				if (obj.directionHorizontal > 0) {
					obj.texture = res.texRunRight;
					obj.currentAnimation = res.ANIM_PLAYER_RUN_RIGHT;
				}
				else if (obj.directionHorizontal < 0) {
					obj.texture = res.texRunLeft;
					obj.currentAnimation = res.ANIM_PLAYER_RUN_LEFT;
				}
				else if (obj.directionVertical < 0) {
					obj.texture = res.texRunUp;
					obj.currentAnimation = res.ANIM_PLAYER_RUN_UP;
				}
				else if (obj.directionVertical > 0) {
					obj.texture = res.texRunDown;
					obj.currentAnimation = res.ANIM_PLAYER_RUN_DOWN;
				}
			}
			break;
		}
		}


		// add acceleration to velocity 
		obj.velocity += obj.acceleration * deltaTime;

		if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
			obj.velocity.x = (obj.velocity.x > 0 ? 1 : -1) * obj.maxSpeedX;
		}
		if (std::abs(obj.velocity.y) > obj.maxSpeedY) {
			obj.velocity.y = (obj.velocity.y > 0 ? 1 : -1) * obj.maxSpeedY;
		}

	}

	// add velocity to position
	obj.position += obj.velocity * deltaTime;

}

auto createObject(int r, int c, SDL_Texture * tex, ObjectType type,const SDLState & state){
	GameObject o;
	o.type = type;

	glm::vec2 isoPos = orthoToIso(c, r, TILE_SIZE, state);
	o.position = isoPos;
	o.texture = tex;

	return o;
};

auto drawAll(short map[MAP_ROWS][MAP_COLS], int r, int c, const SDLState &state, GameState &gs, const Resources &res) {
	switch (map[r][c])
	{
	case 1: {
		GameObject o = createObject(r, c, res.texDirt, ObjectType::level, state);
		o.sprite_width = TILE_SIZE;
		o.sprite_height = TILE_SIZE;
		o.scale = 1.0f;
		gs.layers[LAYER_IDX_LEVEL].push_back(o);
		break;
	}
	case 2: {
		GameObject p = createObject(r, c, res.texGrass, ObjectType::level, state);
		p.sprite_width = TILE_SIZE;
		p.sprite_height = TILE_SIZE;
		p.scale = 1.0f;
		gs.layers[LAYER_IDX_LEVEL].push_back(p);
		break;
	}
	case 3: {
		GameObject player = createObject(r, c, res.texIdleDown, ObjectType::player, state);

		player.data.player = PlayerData();
		player.dynamic = true;
		player.animations = res.playerAnimations;
		player.currentAnimation = res.ANIM_PLAYER_IDLE_DOWN;
		player.maxSpeedX = 50;
		player.maxSpeedY = 50;
		player.sprite_width = 96.0f;
		player.sprite_height = 80.0f;
		player.scale = 0.5f;

		gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
		break;
	}
	case 5: {
		GameObject o = createObject(r, c, res.texDirtPillar, ObjectType::level, state);
		o.sprite_width = TILE_SIZE;
		o.sprite_height = TILE_SIZE;
		o.scale = 1.0f;
		o.solid = true;
		gs.layers[LAYER_IDX_FURNITURE].push_back(o);
		break;
	}
	default: {
		break;

	}
	}
	};

void createTiles(const SDLState& state, GameState& gs, const Resources& res) {
	/*
		tile_003 = 1 = dirt
		tile_040 = 2 = grass
		 - x -	 = 3 = player
		 - x -	 = 4 = enemy
		tile_059 = 5 = dirt pillar
	*/
	short terrain_map[MAP_ROWS][MAP_COLS] =
	{
	{1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1},
	{1,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,2},
	{1,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,2,2},
	{1,1,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2},
	{1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2},
	{1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,2,2},
	{1,1,1,1,1,2,2,1,2,2,1,1,1,1,1,2,2,1,2,2,1,1,1,1,2},
	{1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2},
	{1,1,1,2,2,2,1,1,1,2,1,1,1,2,2,2,1,1,1,2,1,1,2,2,2},
	{1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1},
	{1,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,2},
	{1,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,2,2},
	{1,1,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2},
	{1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2},
	{1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,2,2},
	{1,1,1,1,1,2,2,1,2,2,1,1,1,1,1,2,2,1,2,2,1,1,1,1,2},
	{1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,2,2},
	{1,1,1,2,2,2,1,1,1,2,1,1,1,2,2,2,1,1,1,2,1,1,2,2,2},
	{1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1},
	{1,1,1,1,2,2,2,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,2},
	{1,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,2,2},
	{1,1,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2},
	{1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,2}
	};


	short player_map[MAP_ROWS][MAP_COLS] =
	{
	{3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	short furniture_map[MAP_ROWS][MAP_COLS] =
	{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5}
	};




	for (int r = 0; r < MAP_ROWS; r++) {
		for (int c = 0; c < MAP_COLS; c++) {
			//drawAll(short map[MAP_ROWS][MAP_COLS], int r, int c, SDLState & state, GameState & gs, Resources & res);
			drawAll(terrain_map, r, c, state, gs, res);
			drawAll(player_map, r, c, state, gs, res);
			drawAll(furniture_map, r, c, state, gs, res);

		}
	}
}

