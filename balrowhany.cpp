#include "fonts.h"
#include <iostream>
#include <X11/keysym.h>
#include "balrowhany.h"
#include "mchitorog.h"
#include "jsandoval.h"
//#include <GL/glx.h>

using namespace std;


int handle_menu_keys(int key, GameState &state, int &selected_option)
{
    if (state == MENU) {
        switch(key) {
            case XK_Up:
                // Menu navigation sound
                playGameSound(MENU_CLICK);
                // Assuming 3 options
                selected_option = (selected_option - 1 + 3) % 3;
                break;
            case XK_Down:
                // Menu navigation sound
                playGameSound(MENU_CLICK);
                selected_option = (selected_option + 1) % 3;
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
        }
    }
    
    return 0;
}



void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture, int selected_option){
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
        Rect title;
        title.left = xres/2;
        title.bot = yres - 200;
        title.center = 1;
        
        ggprint40(&title, 50, 0x867933, "SPACE  PIRATES");
        title.bot = yres - 198;
        ggprint40(&title, 50, 0x2C2811, "SPACE  PIRATES");
        title.bot = yres - 200;
        ggprint40(&title, 50, 0xDBAD6A, "SPACE  PIRATES");

        //  menu options
        Rect r;
        r.left = xres/2;
        r.bot = yres - 270;
        r.center = 1;
        
        const char* options[] = {"START", "CONTROLS", "EXIT"}; // menu option color 
        for (int i = 0; i < 3; i++) {
            int color = (i == selected_option) ? 0x00FF99FF : 0x00FFFFFF;
            ggprint17(&r, 40, color, options[i]);
        }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    EnableFor3D();
}; 


void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture){
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

    // controls 
    Rect r;
    r.left = xres/2;
    r.bot = yres - 150;
    r.center = 1;

    ggprint40(&r, 30, 0x00FF99FF, "CONTROLS");
    r.bot = yres/2;
    ggprint17(&r, 30, 0x00ffffff, "WASD - move ship");
    ggprint17(&r, 30, 0x00ffffff, "SPACE - tbd");
    ggprint17(&r, 30, 0x00ffffff, "E - interact");
    ggprint17(&r, 30, 0x00ffffff, "ESC - exit/menu");

    EnableFor3D();
};


// bar

void initializeEntity(ecs::Entity* spaceship)
{

 //   auto spaceship = ecs::ecs.entity().checkout(); // create 
    auto health = ecs::ecs.component().assign<ecs::Health>(spaceship);
    auto oxygen = ecs::ecs.component().assign<ecs::Oxygen>(spaceship);
    auto fuel = ecs::ecs.component().assign<ecs::Fuel>(spaceship);
    auto transform = ecs::ecs.component().assign<ecs::Transform>(spaceship);

    if (health) {
        health -> health = 50.0f; 
        health -> max = 100.0f;
      //  drawUIBar("Health", getHealth->health, getHealth->max, 20, 500, 0xFF0000);
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

    auto getHealth = ecs::ecs.component().fetch<ecs::Health>(spaceship);
    auto getOxygen = ecs::ecs.component().fetch<ecs::Oxygen>(spaceship);
    auto getFuel = ecs::ecs.component().fetch<ecs::Fuel>(spaceship);
    auto getTransform = ecs::ecs.component().fetch<ecs::Transform>(spaceship);

    cout << "Spaceship intialized with health, oxygen, fuel, and transform components." << endl;
    if (getHealth) {
        cout << "Health: " << getHealth -> health << " / " << getHealth -> max << endl;
        float healthPercent = (getHealth -> health / getHealth -> max) * 100.0f;
      cout << "Health Percentage: " << healthPercent << "%" << endl << endl;
 
    } else {
        cout << "Health component not found." << endl;
    }

    if (getOxygen) {
        cout << "Oxygen: " << getOxygen -> oxygen << " / " << getOxygen -> max << endl;
    } else {
        cout << "Oxygen component not found." << endl;
    }

    if (getFuel) {
        cout << "Fuel: " << getFuel -> fuel << " / " << getFuel -> max << endl;
    } else {
        cout << "Fuel component not found." << endl;
    }

    if (getTransform) {
        cout << "Position: " << getTransform -> pos[0] << ", " << getTransform -> pos[1] << endl;
    } else {
        cout << "Transform component not found." << endl;
    }
    
}


//draw
void drawUIBar (const char* label, float current, float max, float x, float y, unsigned int color) 
{
    float percentage = (current / max);  //used to dynamically display bar 
    int barWidth = 150;
    int barHeight = 8;

    Rect r;
        r.left = x + 60;   //spacing between bars
        r.bot = y + barHeight + 5; //space between label and bar
        ggprint13(&r, 16, color, "%s: %0.f/%0.f", label, current, max);

    // background bar (dark grey)
    glColor3f(0.1f, 0.1f, 0.1f);  // black bg for empty portion
    glBegin(GL_QUADS); 
        glVertex2f(x, y); // 2d vertex 
        glVertex2f(x + barWidth, y); // 
        glVertex2f(x + barWidth, y + barHeight); 
        glVertex2f(x, y + barHeight);
    glEnd();
   
    // forground bar (dynamic color fill)
    glColor3f(0.5f,0.5f, 0.5f); 
    glBegin(GL_QUADS); 
        glVertex2f(x, y); //finish
        glVertex2f(x + (barWidth * percentage), y);
        glVertex2f(x + (barWidth * percentage), y + barHeight); 
        glVertex2f(x, y + barHeight); 
    glEnd(); 


}