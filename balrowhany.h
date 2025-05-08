#pragma once
#include <GL/glx.h>
#include "jlo.h"

#define OXYGEN ecs::Oxygen
#define FUEL ecs::Fuel
#define ASTEROID ecs::Asteroid
#define COLLECTIBLE ecs::Collectible

// Game States for Menu Logic
enum GameState {
    MENU,          // 0 
    PLAYING,       // 1
    CONTROLS,      // 2 
    CREDITS,       // 3
    SPACE,         // 4
    GAMEOVER,      // 5 
    EXIT,          // 6
    PAUSED,        // 7
    SPLASH         // 8     
};

//ecs Components
namespace ecs {
    struct Oxygen {
        float oxygen;
        float max;
    };

    struct Fuel {
        float fuel;
        float max;
    };

     struct Asteroid {
        float size; 
        float rotationSpeed; 
        float health; 
        float movementSpeed;
        bool exploding = false;
        int direction; // 1-4 directions 5-8 corners <-- replace SpawnPoint


    };

    //struct SpawnPoint {
      //  int direction; // 1-4 directions 5-8 corners  
    //};

    struct Collectible {
        int type;  // 0 = oxygen 1 = fuel
        int value;  //resource increment of oxygen or fuel
    };
}

//Menu
int handle_menu_keys(int key, GameState &state, int &selected_option);
void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture, GLuint titleTexture, int selected_option);
void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture, GameState controls_from_state);

//Bar
void initializeEntity(ecs::Entity* entity);
void drawUIBar(const char* label, float current, float max, float x, float y, unsigned int color);

//Asteroids
void SampleSpaceEntities();
ecs::Entity* createAsteroid(float x, float y);
void generateAsteroids(int count, [[maybe_unused]]int xres, [[maybe_unused]]int yres, ecs::Entity* spaceship);
bool checkCircleCollision(const ecs::Entity* spaceship, const ecs::Entity* asteroid); 
void moveAsteroids(ecs::Entity* spaceship);
void spawnAsteroids(ecs::Entity* spaceship, int xres, int yres);

void decrementResources(GameState &state, ecs::Entity* spaceship); 

float getDeltaTime(); 


ecs::Entity* createCollectible(float x, float y);

void spawnCollectibles(ecs::Entity* spaceship, int xres, int yres);

bool collectiblePickedUp(ecs::Entity* spaceship, const ecs::Entity* collectible);
void handleCollectiblePickup(ecs::Entity* spaceship, const ecs::Entity* collectible);
void handleCollectibleInteractions(ecs::Entity* spaceship); 