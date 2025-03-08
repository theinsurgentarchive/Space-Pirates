#pragma once
#include <GL/glx.h>

// Game states
enum GameState {
	MENU, //  0 
	PLAYING, // 1
	CONTROLS,  // 2 
	EXIT 
};


int handle_menu_keys(int key, GameState &state, int &selected_option); 
void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture, int selected_option);
void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture); 