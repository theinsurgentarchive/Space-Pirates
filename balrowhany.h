#pragma once
#include <GL/glx.h>
#include "jlo.h"

#define OXYGEN ecs::Oxygen
#define FUEL ecs::Fuel
#define ASTEROID ecs::Asteroid

// Game States for Menu Logic
enum GameState {
    MENU,       // 0 
    PLAYING,    // 1
    CONTROLS,   // 2 
    CREDITS,    // 3
    SPACE,      // 4
    GAMEOVER,   // 5 
    EXIT        // 6
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

    };

    struct SpawnPoint {
        int direction; // 1-4 directions 5-8 corners  
    };
}

//Menu
int handle_menu_keys(int key, GameState &state, int &selected_option);
void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture, GLuint titleTexture, int selected_option);
void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture);

//Bar
void initializeEntity(ecs::Entity* entity);
void drawUIBar(const char* label, float current, float max, float x, float y, unsigned int color);

//Asteroids
void SampleSpaceEntities();
ecs::Entity* createAsteroid(float x, float y);
void generateAsteroids(int count, [[maybe_unused]]int xres, [[maybe_unused]]int yres);
bool checkCircleCollision(const ecs::Entity* spaceship, const ecs::Entity* asteroid); 
void moveAsteroids(ecs::Entity* spaceship);
void spawnAsteroids(ecs::Entity* spaceship, int xres, int yres);

void decrementResources(GameState &state, ecs::Entity* spaceship); 