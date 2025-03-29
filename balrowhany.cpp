#include "fonts.h"
#include <iostream>
#include <X11/keysym.h>
#include "balrowhany.h"
#include "mchitorog.h"
#include "jsandoval.h"

using namespace std;

int handle_menu_keys(int key, GameState &state, int &selected_option)
{
	if (state == MENU) {
		switch(key) {
			case XK_Up:
				// Menu navigation sound
				playGameSound(MENU_CLICK);
				// Now 4 options (Start, Controls, Credits, Exit)
				selected_option = (selected_option - 1 + 4) % 4;
				break;
			case XK_Down:
				// Menu navigation sound
				playGameSound(MENU_CLICK);
				selected_option = (selected_option + 1) % 4;
				break;
			case XK_Return:
				// Menu selection sound
				playGameSound(MENU_CLICK);

				if (selected_option == 0) {
					// Start game option
					state = PLAYING;
					// Update audio to match the new game state
					updateAudioState(state);
				} else if (selected_option == 1) {
					// Controls option
					state = CONTROLS;
					// Update audio to match the new game state
					updateAudioState(state);
				} else if (selected_option == 2) {
					// Credits option
					state = CREDITS;
					// Update audio to match the new game state
					updateAudioState(state);
				} else if (selected_option == 3) {
					// Exit option
					return 1; // Exit request
				}
				break;
			case XK_Escape:
				return 1; // Exit request
		}
	} else if (state == CONTROLS) {
		if (key == XK_Escape || key == XK_Return) {
			// Play sound when returning to menu
			playGameSound(MENU_CLICK);
			state = MENU;
			// Update audio to match the new game state
			updateAudioState(state);
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

void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture, GLuint titleTexture, int selected_option) {
	DisableFor2D();

	glPushMatrix();
	// Draw background
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

	// Draw title texture

	// Draw title texture
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();
	int titleWidth = 300; // Adjust based on your actual texture size
	int titleHeight = 300; // Adjust based on your actual texture size
	int titleX = xres / 2 - titleWidth / 2;
	int titleY = yres - 150 - titleHeight / 2;

	glBindTexture(GL_TEXTURE_2D, titleTexture);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Ensure full alpha for the texture
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(titleX, titleY);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(titleX, titleY + titleHeight);
	glTexCoord2f(1.0f, 0.0f); glVertex2i(titleX + titleWidth, titleY + titleHeight);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(titleX + titleWidth, titleY);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	glDisable(GL_BLEND);


	// Menu options
	Rect r;
	r.left = xres/2;
	r.bot = yres - 350;
	r.center = 1;

	const char* options[] = {"START", "CONTROLS", "CREDITS", "EXIT"}; // menu option color 
	for (int i = 0; i < 4; i++) {
		int color = (i == selected_option) ? 0x00FF99FF : 0x00FFFFFF;
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

	EnableFor3D();
}

void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture) {
    DisableFor2D();
    
    // Controls screen background
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

    // Controls title - position at 10% from top
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
    ggprint13(&r, returnSize, 0xFFFFFFFF, "Press ESC or ENTER to return to menu");

    EnableFor3D();
}

void initializeEntity(ecs::Entity* spaceship)
{
	auto health = ecs::ecs.component().assign<ecs::Health>(spaceship);
	auto oxygen = ecs::ecs.component().assign<ecs::Oxygen>(spaceship);
	auto fuel = ecs::ecs.component().assign<ecs::Fuel>(spaceship);
	auto transform = ecs::ecs.component().assign<ecs::Transform>(spaceship);

	if (health) {
		health->health = 50.0f;
		health->max = 100.0f;
	}

	if (transform) {
		transform->pos[0] = 40.0f;
		transform->pos[1] = 60.0f;
	}

	if (fuel) {
		fuel->fuel = 150.0f;
		fuel->max = 300.0f;
	}

	if (oxygen) {
		oxygen->oxygen = 100.0f;
		oxygen->max = 200.0f;
	}

	// Debug output if needed
	[[maybe_unused]] auto getHealth = ecs::ecs.component().fetch<ecs::Health>(spaceship);
	[[maybe_unused]] auto getOxygen = ecs::ecs.component().fetch<ecs::Oxygen>(spaceship);
	[[maybe_unused]] auto getFuel = ecs::ecs.component().fetch<ecs::Fuel>(spaceship);
	[[maybe_unused]] auto getTransform = ecs::ecs.component().fetch<ecs::Transform>(spaceship);

	cout << "Spaceship initialized with health, oxygen, fuel, and transform components." << endl;
}

// Sixth function: drawUIBar
void drawUIBar(const char* label, float current, float max, float x, float y, unsigned int color)
{
	float percentage = (current / max);
	int barWidth = 150;
	int barHeight = 8;

	Rect r;
	r.left = x + 60;
	r.bot = y + barHeight + 5;
	ggprint13(&r, 16, color, "%s: %0.f/%0.f", label, current, max);

	// background bar (dark grey)
	glColor3f(0.1f, 0.1f, 0.1f);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + barWidth, y);
	glVertex2f(x + barWidth, y + barHeight);
	glVertex2f(x, y + barHeight);
	glEnd();

	// foreground bar (dynamic color fill)
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + (barWidth * percentage), y);
	glVertex2f(x + (barWidth * percentage), y + barHeight);
	glVertex2f(x, y + barHeight);
	glEnd();
}
