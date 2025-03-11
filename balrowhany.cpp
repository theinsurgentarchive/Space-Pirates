#include "fonts.h"
#include <iostream>
#include <X11/keysym.h>
#include "balrowhany.h"
#include "mchitorog.h"
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


}; 


void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture){
    glPushMatrix();
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
    glPopMatrix();

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
};
