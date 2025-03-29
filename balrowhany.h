#pragma once
#include <GL/glx.h>
#include "jlo.h"

#define OXYGEN ecs::Oxygen
#define FUEL ecs::Fuel

// Game states for menu logic
enum GameState {
    MENU,       // 0 
    PLAYING,    // 1
    CONTROLS,   // 2 
    CREDITS,    // 3
    EXIT        // 4
};

// ecs components 
namespace ecs {
    struct Oxygen {
        float oxygen;
        float max;
    };

    struct Fuel {
        float fuel;
        float max;
    };
}

// menu 
int handle_menu_keys(int key, GameState &state, int &selected_option); 
void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture, GLuint titleTexture, int selected_option);
void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture); 

// bar
void initializeEntity(ecs::Entity* entity); 
void drawUIBar(const char* label, float current, float max, float x, float y, unsigned int color);
