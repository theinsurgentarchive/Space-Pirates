#include "fonts.h"
#include <iostream>
#include <cmath>
#include <X11/keysym.h>
#include "balrowhany.h"
#include "mchitorog.h"
#include "jsandoval.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

extern const Camera* spaceCamera; //spaceCamera in main




int handle_menu_keys(int key, GameState &state, int &selected_option)
{
	if (state == MENU) {
		switch(key) {
			case XK_Up:
				playGameSound(MENU_CLICK);
				selected_option = (selected_option - 1 + 5) % 5;
				break;
			case XK_Down:
				playGameSound(MENU_CLICK);
				selected_option = (selected_option + 1) % 5;
				break;
			case XK_Return:
				playGameSound(MENU_CLICK);
				return selected_option + 10;
				/*if (selected_option == 0) {
				  state = PLAYING;
				  updateAudioState(state);
				  } else if (selected_option == 1) {
				  state = CONTROLS;
				  updateAudioState(state);
				  } else if (selected_option == 2) {
				  state = CREDITS;
				  updateAudioState(state);
				  } else if (selected_option == 3) {
				  state = SPACE;
				  updateAudioState(state);
				  } else if (selected_option == 4) {
				// Exit option
				return 1; // Exit request
				}*/
				break;
			case XK_Escape:
				return 1;
		}
	} else if (state == CONTROLS) {
		if (key == XK_Escape || key == XK_Return) {
			playGameSound(MENU_CLICK);
			/*state = MENU;
			  updateAudioState(state);*/
			return -1;
		} else if (key == XK_m || key == XK_M) {
			// Toggle music
			playGameSound(MENU_CLICK);
			getAudioManager()->toggleMusic();
		} else if (key == XK_s || key == XK_S) {
			// Toggle sound effects
			getAudioManager()->toggleSound();
			// Only play click if sound is enabled
			if (getAudioManager()->isSoundEnabled()) {
				playGameSound(MENU_CLICK);
			}
		}
	} else if (state == CREDITS) {
		if (key == XK_Escape || key == XK_Return) {
			// Play sound when returning to menu
			playGameSound(MENU_CLICK);
			state = MENU;
			// Update audio to match the new game state
			updateAudioState(state);
		}
	}

	return 0;
}



void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture,  
		[[maybe_unused]]  GLuint titleTexture, int selected_option) {
	DisableFor2D();

	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, menuBackgroundTexture);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(0, yres);
	glTexCoord2f(1.0f, 0.0f); glVertex2i(xres, yres);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(xres, 0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	// menu title
	/*Rect title;
	  title.left = xres/2;
	  title.bot = yres - 200;
	  title.center = 1;

	  ggprint40(&title, 50, 0x867933, "SPACE  PIRATES");
	  title.bot = yres - 198;
	  ggprint40(&title, 50, 0x2C2811, "SPACE  PIRATES");
	  title.bot = yres - 200;
	  ggprint40(&title, 50, 0xDBAD6A, "SPACE  PIRATES");
	  */

	// Draw title logo instead of text
	render_title_logo(xres, yres, titleTexture);

	//  menu options
	Rect r;
	r.left = xres/2;
	r.bot = yres - 270;
	r.center = 1;


	const char* options[] = {"START", "CONTROLS", "CREDITS", "SPACE",
		"EXIT"}; 
	for (int i = 0; i < 5; i++) {
		int color = (i == selected_option) ? 0x00FF99FF : 0x00FFFFFF;
		// menu option color change
		ggprint17(&r, 40, color, options[i]);
	}

	// Audio status indicators
	Rect audioStatus;
	audioStatus.left = 20;
	audioStatus.bot = 60;
	audioStatus.center = 0;

	AudioManager* audioMgr = getAudioManager();
	if (audioMgr->isMusicEnabled()) {
		ggprint13(&audioStatus, 20, 0x00FFFFFF, "MUSIC: ON");
	} else {
		ggprint13(&audioStatus, 20, 0x00999999, "MUSIC: OFF");
	}

	audioStatus.bot -= 5;
	if (audioMgr->isSoundEnabled()) {
		ggprint13(&audioStatus, 20, 0x00FFFFFF, "SFX: ON");
	} else {
		ggprint13(&audioStatus, 20, 0x00999999, "SFX: OFF");
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	EnableFor3D();
}; 


void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture, GameState controls_from_state) {
	DisableFor2D();
	//  controls screen bg
	glBindTexture(GL_TEXTURE_2D, menuBackgroundTexture);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(0, yres);
	glTexCoord2f(1.0f, 0.0f); glVertex2i(xres, yres);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(xres, 0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	// Determine font size based on window dimensions
	int titleSize = 40;
	int headingSize = 17;
	int textSize = 14;
	int returnSize = 13;

	if (xres < 800 || yres < 600) {
		titleSize = 30;
		headingSize = 14;
		textSize = 12;
		returnSize = 10;
	}

	// controls 
	Rect r;
	r.left = xres/2;
	r.bot = yres - (yres * 0.1);
	r.center = 1;

	ggprint40(&r, titleSize, 0xFF00FF00, "CONTROLS");

	// Game controls section
	float gameControlsStartY = 0.25;
	r.bot = yres - (yres * gameControlsStartY);
	ggprint17(&r, headingSize, 0xFFFFFFFF, "GAME CONTROLS");

	// Controls list - game controls
	float controlSectionHeight = yres * 0.25;
	float firstControlY = 0.30; // Default is 0.25 from top
	int numControls = 5;  // Number of control entries
	int controlSpacing = (controlSectionHeight / numControls) * 0.6;
	r.bot = yres - (yres * firstControlY);
	ggprint17(&r, textSize, 0xFF7DF9FF, "WASD - Move ship");
	r.bot -= controlSpacing;
	ggprint17(&r, textSize, 0xFF7DF9FF, "SPACE - Fire");
	r.bot -= controlSpacing;
	ggprint17(&r, textSize, 0xFF7DF9FF, "E - Interact");
	r.bot -= controlSpacing;
	ggprint17(&r, textSize, 0xFF7DF9FF, "ESC - Exit/Menu");
	r.bot -= controlSpacing;
	ggprint17(&r, textSize, 0xFF7DF9FF, "Mouse - Aim");

	// Audio controls section
	r.bot = yres - (yres * 0.65);
	ggprint17(&r, headingSize, 0xFFFFFFFF, "AUDIO CONTROLS");

	// Audio manager status check
	AudioManager* audioMgr = getAudioManager();

	// Controls list - audio controls
	r.bot = yres - (yres * 0.70);

	if (audioMgr->isMusicEnabled()) {
		ggprint17(&r, textSize, 0xFF7DF9FF, "M - Toggle Music [ON]");
	} else {
		ggprint17(&r, textSize, 0xFF999999, "M - Toggle Music [OFF]");
	}

	r.bot -= (controlSpacing) * 0.8;

	if (audioMgr->isSoundEnabled()) {
		ggprint17(&r, textSize, 0xFF7DF9FF, "S - Toggle Sound Effects [ON]");
	} else {
		ggprint17(&r, textSize, 0xFF999999, "S - Toggle Sound Effects [OFF]");
	}

	// Return instructions - position at 5% from bottom
	r.bot = yres * 0.05;
	if (controls_from_state == PAUSED) {
		ggprint13(&r, returnSize, 0xFFFFFFFF, "Press ESC or ENTER to return to pause menu");
	} else {
		ggprint13(&r, returnSize, 0xFFFFFFFF, "Press ESC or ENTER to return to menu");
	}

	EnableFor3D();
}

// bar

void initializeEntity(ecs::Entity* spaceship)
{
	auto [health,oxygen,fuel,transform,sprite,physics] = ecs::ecs.component().assign<HEALTH,ecs::Oxygen,ecs::Fuel,TRANSFORM,SPRITE,PHYSICS>(spaceship);

	//default values
	transform->pos = {50, 50};  //
	if (sprite && physics) {
		sprite->ssheet = "ship-right"; 
		physics ->vel = {0.0f, 0.0f}; //not moving 
	}




	if (health) {
		health -> health = 5.0f; 
		health -> max = 100.0f;

	}

	if (transform) {
		transform -> pos[0] = 40.0f;
		transform -> pos[1] = 60.0f; 
	}

	if (fuel) {
		fuel -> fuel = 150.0f; 
		fuel -> max = 300.0f; 
	}

	if (oxygen) {
		oxygen -> oxygen = 100.0f;
		oxygen -> max = 200.0f;
	}

	auto [getHealth,getOxygen,getFuel,getTransform] = ecs::ecs.component().fetch<HEALTH,ecs::Oxygen,ecs::Fuel,TRANSFORM>(spaceship);

	//cout << "Spaceship intialized with health, oxygen, & fuel" << endl;
	if (getHealth) {
		DINFOF("Health: %.2f / %.2f\n", 
				getHealth -> health <<  getHealth -> max);  
		[[maybe_unused]] float healthPercent = 
			(getHealth -> health / getHealth -> max) * 100.0f;
		DINFOF("Health Percentage: %.2f%%\n", healthPercent);

	} else {
		DINFO("Health component not found.\n");
	}

	if (getOxygen) {
		DINFOF("Oxygen: %.2f / %.2f\n", getOxygen->oxygen, getOxygen->max);
	} else {
		DINFO("Oxygen component not found.\n");
	}

	if (getFuel) {
		DINFOF("Fuel: %.2f / %.2f\n", getFuel->fuel, getFuel->max);
		DINFOF("Fuel component not found."); 
	}

	if (getTransform) {
		DINFOF("Position: %.2f, %.2f\n",
				getTransform->pos[0], getTransform->pos[1]);
	} else {
		DINFO("Transform component not found.\n");
	}

	DINFOF("\n");

}


//draw bar
void drawUIBar (const char* label, float current, float max, 
		float x, float y, unsigned int color) 
{
	float percentage = (current / max);  //used to dynamically display bar 
	int barWidth = 150;
	int barHeight = 8;

	Rect r;
	r.left = x + 60;   //spacing between bars
	r.bot = y + barHeight + 5; //space between label and bar
	ggprint13(&r, 16, color, "%s: %0.f/%0.f", label, current, max);

	// background bar (dark grey) -chat help
	glColor3f(0.1f, 0.1f, 0.1f);  // black bg for empty portion
	glBegin(GL_QUADS); 
	glVertex2f(x, y); // 2d vertex 
	glVertex2f(x + barWidth, y); // 
	glVertex2f(x + barWidth, y + barHeight); 
	glVertex2f(x, y + barHeight);
	glEnd();

	// forground bar (dynamic color fill) -chat help
	glColor3f(0.5f,0.5f, 0.5f); 
	glBegin(GL_QUADS); 
	glVertex2f(x, y); //finish
	glVertex2f(x + (barWidth * percentage), y);
	glVertex2f(x + (barWidth * percentage), y + barHeight); 
	glVertex2f(x, y + barHeight); 
	glEnd(); 


}


// Load asteroid and ship related sprites into custom sprite sheet map.
void loadShipAndAsteroids(
		std::unordered_map<std::string,
		std::shared_ptr<SpriteSheet>>& shipAndAsteroidsSheets)
{
	SpriteSheetLoader loader {shipAndAsteroidsSheets};  
	//loader instance using custom map defined above
	DINFOF("Loading asteroid base.png sprites...\n") 
		loader


		.loadStatic("asteroid", 
				loadTexture(
					"./resources/textures/space/base.png", true), {1,1}, {24,24}) 
		.loadStatic("asteroid-explode", 
				loadTexture(
					"./resources/textures/space/explode.png", true), {1,8}, {32,32}, true)
		.loadStatic("ship-front-back", 
				loadTexture(
					"./resources/textures/space/ship-front-back.png", true),
				{1,1}, {32,32})
	.loadStatic("ship-left", 
			loadTexture(
				"./resources/textures/space/ship-left.png", true), {1,1}, {32,32})
		.loadStatic("ship-right", 
				loadTexture(
					"./resources/textures/space/ship-right.png", true), {1,1}, {32,32});
	DINFOF("finished loading asteroid and ship sprites.\n");
}


// creates a asteroid with it's properties 

ecs::Entity* createAsteroid(float x, float y)  
{
	ecs::Entity* asteroid = ecs::ecs.entity().checkout();

	if (!asteroid) {
		DINFOF("Failed to create asteroid entity.\n");
		return nullptr;
	}

	//assign Asteroid properties
	float size = (float)(rand() % 10 + 1); // random size between 1.0 and 10.0
	auto [singleAsteroid,transform,sprite] = ecs::ecs.component().assign<ecs::Asteroid,ecs::Transform,ecs::Sprite>(asteroid);
	singleAsteroid->health = size * 2; // based on size
	singleAsteroid->size = size; // 1, 2, or 3

	//singleAsteroid->rotationSpeed = (rand() % 100) / 100.0f; // random rotation speed between 0 and 1
	singleAsteroid->movementSpeed = static_cast<float>((rand() % 600) + 50); // speed range 50-549

	// assign Transform properties
	float scaleSize = static_cast<float>((rand() % 8) + 1); // 1 to 8
	transform->pos = {x, y};
	transform->scale = {scaleSize, scaleSize}; //{size, size};
	transform->rotation = 0.0f;
	// assign Sprite properties
	sprite->ssheet = "asteroid";
	sprite->frame = 0;

	[[maybe_unused]]auto physics = ecs::ecs.component().assign<ecs::Physics>(asteroid);
	//cout << "Asteroid created with sprite: " << sprite->ssheet << ", frame: " << sprite->frame << endl;
	return asteroid;
}

void generateAsteroids(int count, int xres, int yres, ecs::Entity* spaceship) 

{
	auto [transform] = ecs::ecs.component().fetch<TRANSFORM>(spaceship);

	float CameraX = transform->pos[0];
	float CameraY = transform->pos[1];


	std::cout << "Camera pos: (" << CameraX << ", " << CameraY << ")\n";



	for (int i = 0; i < count; i++) {
		float x = 0.0f; 
		float y = 0.0f; 


		int directionRandomizer = (rand() % 8) + 1; // 1 to 4 


		//float x = CameraX + (xres / 2) + (rand() % 200); 
		// just outside right edge
		//float y = CameraY - (yres / 2) + (rand() % yres); 
		// full vertical spread

		switch(directionRandomizer) {   //chat helped +- formula 
			case 1: //top->moving down
				x = CameraX - (xres / 2) + (rand() % 200); 
				y = CameraY + (yres / 2) + (rand() % yres);
				break; 
			case 2: //right->moving left 
				x = CameraX + (xres / 2) + (rand() % 200); 
				y = CameraY - (yres / 2) + (rand() % yres);
				break;
			case 3: //bottom ->moving top 
				x = CameraX - (xres / 2) + (rand() % 200); 
				y = CameraY - (yres / 2) + (rand() % yres);
				break; 
			case 4: //left -> moving right
				x = CameraX - (xres / 2) + (rand() % 200); 
				y = CameraY - (yres / 2) + (rand() % yres);
				break;  

				//corners 

			case 5: //top left 
				x = CameraX - (xres / 2) - (rand() % 200); 
				y = CameraY + (yres / 2) + (rand() % yres);
				break;

			case 6: // top right 
				x = CameraX + (xres / 2) + (rand() % 200); 
				y = CameraY + (yres / 2) + (rand() % yres);
				break; 

			case 7:  // bottom left 
				x = CameraX - (xres / 2) - (rand() % 200); 
				y = CameraY - (yres / 2) - (rand() % yres);
				break;

			case 8: //button right
				x = CameraX + (xres / 2) + (rand() % 200); 
				y = CameraY - (yres / 2) - (rand() % yres);
				break;         
		}
		//create Asteroid entity with it's properties in x, y position
		std::cout << "Spawning asteroid at: (" << x << ", " << y << ")\n"; //test1
		ecs::Entity* asteroid = createAsteroid(x, y); 

		// assign that Asteroid with spawnPoint for randomized direction 
		auto [spawnPoint] = ecs::ecs.component().assign<ecs::SpawnPoint>(asteroid);
		spawnPoint->direction = directionRandomizer; 

		DINFOF("Spawning asteroid at x: %.2f, y: %.2f (dir %d)\n", x, y, 
				directionRandomizer);




		//  createAsteroid(x, y); // create asteroid at random x,y positions
	}
}

void spawnAsteroids(ecs::Entity* spaceship, int xres, int yres) {
	// auto-spawn asteroids every 5 seconds
	static auto lastAsteroidSpawn = 
		std::chrono::high_resolution_clock::now();
	auto current = std::chrono::high_resolution_clock::now();
	auto secondsPassed = std::chrono::duration_cast<std::chrono::seconds>(current - lastAsteroidSpawn);

	if (secondsPassed.count() >= 3) {  
		DINFOF("Spawning Asteroids \n");
		cout << "Spawning asteroid" << endl;
		[[maybe_unused]]auto transform = ecs::ecs.component().fetch<TRANSFORM>(spaceship);
		generateAsteroids(rand() % 2 + 4, xres, yres, spaceship); //spawn count 
		lastAsteroidSpawn = current; //reset timer
	}
	// set asteroids w/ asteroid sprite
	auto spaceEntities = ecs::ecs.query<SPRITE, TRANSFORM, ASTEROID>();
	for (auto* entity : spaceEntities) {
		auto [sprite] = ecs::ecs.component().fetch<SPRITE>(entity);
		if (sprite) {
			sprite->ssheet = "asteroid";
			//cout << "set space sprite for asteroid entity" << endl;
		}
	}

	if (spaceship == nullptr){
		DINFOF("Error: Spaceship Null!\n");
		return; //debug 
	}

	moveAsteroids(spaceship);


}

// collision


bool checkCircleCollision(const ecs::Entity* spaceship, const ecs::Entity* asteroid) {
	auto [spaceshipTransform] = ecs::ecs.component().fetch<ecs::Transform>(spaceship);
	auto [asteroidTransform] = ecs::ecs.component().fetch<ecs::Transform>(asteroid);


	if (!spaceshipTransform || !asteroidTransform){
		DINFOF("We are missing components for collision");
		return false; 
	}

	float dx = spaceshipTransform->pos[0] - asteroidTransform->pos[0];
	float dy = spaceshipTransform->pos[1] - asteroidTransform->pos[1];
	float distance = sqrt(pow(dx, 2) + pow(dy, 2)); 

	float spaceshipRadius = 30.0f;  
	float asteroidRadius = 20.0f; // temp, get real size / 2

	return distance < (spaceshipRadius + asteroidRadius); 
	//circle collision formula
}




void moveAsteroids(ecs::Entity* spaceship) 
{
	if (!spaceship) {
		DINFOF("Spaceship is null\n");
		return; 
	}

	float dt = getDeltaTime(); 


	auto asteroids = ecs::ecs.query<ecs::Asteroid, ecs::Transform>(); 
	for (auto* asteroid: asteroids) {  // chat: how loop to through all asteroids. 
		auto [transform, asteroidComp, sprite, spawnDirection] =
			ecs::ecs.component().fetch<ecs::Transform, ecs::Asteroid, ecs::Sprite, 
			ecs::SpawnPoint>(asteroid);

		if (!transform || !asteroidComp || !sprite || !spawnDirection)
			continue; 
		// chat did the math 
		switch (spawnDirection->direction){
			case 1: transform->pos[1] -= asteroidComp->movementSpeed * dt ; break; 
				// top->down
			case 2: transform->pos[0] -= asteroidComp->movementSpeed * dt; break; 
				// right-left
			case 3: transform->pos[1] += asteroidComp->movementSpeed * dt; break;
				// bottom-up
			case 4: transform->pos[0] += asteroidComp->movementSpeed * dt; break; 
				// left-right

				// corners 
			case 5: // top-left -> move down-right //chat formula 
				transform->pos[0] += asteroidComp->movementSpeed * 0.6f * dt;
				transform->pos[1] -= asteroidComp->movementSpeed * 0.6f * dt;
				break;
			case 6:  // top-right -> move down-left
				transform->pos[0] -= asteroidComp->movementSpeed * 0.6f * dt;
				transform->pos[1] -= asteroidComp->movementSpeed * 0.6f * dt;
				break;
			case 7: // bottom-left -> move up-right
				transform->pos[0] += asteroidComp->movementSpeed * 0.6f * dt;
				transform->pos[1] += asteroidComp->movementSpeed * 0.6f * dt;
				break;
			case 8: // bottom-right -> move up-left
				transform->pos[0] -= asteroidComp->movementSpeed * 0.6f * dt;
				transform->pos[1] += asteroidComp->movementSpeed * 0.6f * dt;
				break;

		}


		if (asteroidComp->exploding) {
			sprite->frame++; // begin incrementing sprite frame 
			DINFOF("Exploding frame is: %d\n", sprite->frame);


			if (sprite->frame >= 10) { //slight cool down
				DINFOF("Asteroid *poof* aka returned\n");
				ecs::ecs.entity().ret(const_cast<ecs::Entity*>(asteroid));

				continue; // skip to next asteroid 
			}

			continue; //skip
		}

		// collision check and health reduction
		if (checkCircleCollision(spaceship, asteroid)) {
			if (asteroidComp->exploding == false) { 
				asteroidComp->exploding = true; //set exploding true 
				DINFOF("Collison Detected!\n") 
					sprite->ssheet = "asteroid-explode";
				sprite->frame = 2;
				if (asteroidComp->exploding) {
					sprite->frame++; // begin incrementing sprite frame 
					DINFOF("Exploding frame is: %d\n", sprite->frame);
				}

				auto [shipHealth] = ecs::ecs.component().fetch<HEALTH>(spaceship);

				if (shipHealth) {
					shipHealth->health -= 1.0f; 
					DINFOF("Spaceship damaged, health is now: %.2f\n", 
							shipHealth->health);
				} else {
					DINFO("no health comp\n");

				}
			}

		}
	}

}



void decrementResources(GameState &state, ecs::Entity* spaceship) 
{
	auto [fuel,oxygen] = ecs::ecs.component().fetch<ecs::Fuel, ecs::Oxygen>(spaceship); 


	if (fuel && fuel->fuel > 0.0f && oxygen && oxygen->oxygen > 0.0f) {
		fuel->fuel -= 0.5f;
		oxygen->oxygen -= 0.5f; 

		if (fuel->fuel < 0.0f || oxygen->oxygen < 0.0f) { 
			fuel->fuel = 0.0f;
			oxygen->oxygen = 0.0f;

			state = GAMEOVER;
		}
	}

}


float getDeltaTime() { //used for smooth movement 

	static auto last = steady_clock::now(); 
	auto old = last;
	last = steady_clock::now(); 
	auto frameTime = last - old; 
	return duration_cast<duration<float>>(frameTime).count(); 

}



