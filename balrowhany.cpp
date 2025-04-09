#include "fonts.h"
#include <iostream>
#include <cmath>
#include <X11/keysym.h>
#include "balrowhany.h"
#include "mchitorog.h"
#include "jsandoval.h"
//?

using namespace std;




int handle_menu_keys(int key, GameState &state, int &selected_option)
{
    if (state == MENU) {
        switch(key) {
            case XK_Up:
                // Menu navigation sound
                playGameSound(MENU_CLICK);
                // Assuming 3 options
                selected_option = (selected_option - 1 + 5) % 5;
                break;
            case XK_Down:
                // Menu navigation sound
                playGameSound(MENU_CLICK);
                selected_option = (selected_option + 1) % 5;
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
                    state = CREDITS;
                    updateAudioState(state);

				} else if (selected_option == 3) {
					state = SPACE;
				} else if (selected_option == 4) {
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



void render_menu_screen(int xres, int yres, GLuint menuBackgroundTexture,   [[maybe_unused]]  GLuint titleTexture, int selected_option) {
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

        
        const char* options[] = {"START", "CONTROLS", "CREDITS", "SPACE", "EXIT"}; 
        for (int i = 0; i < 5; i++) {
            int color = (i == selected_option) ? 0x00FF99FF : 0x00FFFFFF;
                                            // menu option color change
            ggprint17(&r, 40, color, options[i]);
        }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    EnableFor3D();
}; 


void render_control_screen(int xres, int yres, GLuint menuBackgroundTexture) {
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
    auto sprite = ecs::ecs.component().assign<ecs::Sprite>(spaceship);
    auto physics = ecs::ecs.component().assign<ecs::Physics>(spaceship);

    //default values
    transform->pos = {50, 50};  //
    sprite->ssheet = "ship-right"; 
    physics ->vel = {0.0f, 0.0f}; //not moving 

    
    if (health) {
        health -> health = 50.0f; 
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

    auto getHealth = ecs::ecs.component().fetch<ecs::Health>(spaceship);
    auto getOxygen = ecs::ecs.component().fetch<ecs::Oxygen>(spaceship);
    auto getFuel = ecs::ecs.component().fetch<ecs::Fuel>(spaceship);
    auto getTransform = ecs::ecs.component().fetch<ecs::Transform>(spaceship);

    //cout << "Spaceship intialized with health, oxygen, & fuel" << endl;
    if (getHealth) {
        DINFOF("Health: %.2f / %.2f\n", getHealth -> health <<  getHealth -> max);  
       [[maybe_unused]] float healthPercent = (getHealth -> health / getHealth -> max) * 100.0f;
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
        DINFOF("Position: %.2f, %.2f\n", getTransform->pos[0], getTransform->pos[1]);
    } else {
        DINFO("Transform component not found.\n");
    }

	DINFOF("\n");
    
}


//draw bar
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


// Load asteroid and ship related sprites into custom sprite sheet map.
void loadShipAndAsteroids(
    std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& shipAndAsteroidsSheets)
{
    SpriteSheetLoader loader {shipAndAsteroidsSheets};  //loader instance using custom map defined above
    	DINFOF("Loading asteroid base.png sprites...\n") 
    loader
    

    .loadStatic("asteroid", 
        loadTexture(
            "./resources/textures/space/base.png", true), {1,1}, {24,24}) 
    .loadStatic("asteroid-explode", 
        loadTexture(
            "./resources/textures/space/explode.png", true), {1,8}, {32,32})
    .loadStatic("ship-front-back", 
        loadTexture(
            "./resources/textures/space/ship-front-back.png", true), {1,1}, {32,32})
    .loadStatic("ship-left", 
        loadTexture(
            "./resources/textures/space/ship-left.png", true), {1,1}, {32,32})
    .loadStatic("ship-right", 
        loadTexture(
            "./resources/textures/space/ship-right.png", true), {1,1}, {32,32});
    shipAndAsteroidsSheets["asteroid-explode"]->animated = true; //chat
     DINFOF("finished loading asteroid and ship sprites.\n");
}


// single asteroid

ecs::Entity* createAsteroid(float x, float y) 
{
    ecs::Entity* asteroid = ecs::ecs.entity().checkout();

    if (!asteroid) {
        DINFOF("Failed to create asteroid entity.\n");
        return nullptr;
    }

    //assign Asteroid properties
    float size = (float)(rand() % 3 + 1); // random size between 1.0 and 3.0
    auto singleAsteroid = ecs::ecs.component().assign<ecs::Asteroid>(asteroid);
    singleAsteroid->health = size * 2; // based on size
    singleAsteroid->size = size; // 1, 2, or 3
    //singleAsteroid->rotationSpeed = (rand() % 100) / 100.0f; // random rotation speed between 0 and 1
    singleAsteroid->movementSpeed = static_cast<float>((rand() % 10) + 5); // random movement speed between 5 and 15

    // assign Transform properties
    auto transform = ecs::ecs.component().assign<ecs::Transform>(asteroid);
    float scaleSize = static_cast<float>((rand() % 5) + 1); // 1 to 5
    transform->pos = {x, y};
    transform->scale = {scaleSize, scaleSize}; //{size, size};
    transform->rotation = 0.0f;

    // assign Sprite properties
    auto sprite = ecs::ecs.component().assign<ecs::Sprite>(asteroid);
    sprite->ssheet = "asteroid";
    sprite->frame = 0;



    // assign Physics properties later from full screen asteroids
    [[maybe_unused]]auto physics = ecs::ecs.component().assign<ecs::Physics>(asteroid);
    //cout << "Asteroid created with sprite: " << sprite->ssheet << ", frame: " << sprite->frame << endl;
    return asteroid;
}

void generateAsteroids(int count, int xres, int yres) 
{
    for (int i = 0; i < count; i++) {
        float x = (float)(xres + rand() % 100); // spawn off screen right
       // float y = (float)(rand() % yres); // random y pos (will only display top center)
        float y = static_cast<float>(rand() % yres) - (yres / 2.0f); // shift so y=0 is center, chat fix so asteroids throughout. 
        DINFOF("Spawning asteroid at y: %.2f\n", y);


       
        createAsteroid(x, y); // create asteroid at random x,y positions
    }
}

void spawnAsteroids(ecs::Entity* spaceship, int xres, int yres) {
    // auto-spawn asteroids every 5 seconds
			static auto lastAsteroidSpawn = std::chrono::high_resolution_clock::now();
			auto current = std::chrono::high_resolution_clock::now();
			auto secondsPassed = std::chrono::duration_cast<std::chrono::seconds>(current - lastAsteroidSpawn);
			
			if (secondsPassed.count() >= 5) {  
				DINFOF("Spawning Asteroids \n");
				generateAsteroids(rand() % 2 + 8, xres, yres); //rand 0-1, 7-8 asteroids 
				// random 0-1 + 4 asteroids
				lastAsteroidSpawn = current; //reset timer
			}
            // set asteroids w/ asteroid sprite
			auto spaceEntities = ecs::ecs.query<SPRITE, TRANSFORM, ASTEROID>();
			for (auto* entity : spaceEntities) {
				auto sprite = ecs::ecs.component().fetch<SPRITE>(entity);
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
    auto spaceshipTransform = ecs::ecs.component().fetch<ecs::Transform>(spaceship);
    auto asteroidTransform = ecs::ecs.component().fetch<ecs::Transform>(asteroid);

    if (!spaceshipTransform || !asteroidTransform){
        DINFOF("We are missing components for collision");
        return false; 
    }

    float dx = spaceshipTransform->pos[0] - asteroidTransform->pos[0];
    float dy = spaceshipTransform->pos[1] - asteroidTransform->pos[1];
    float distance = sqrt(pow(dx, 2) + pow(dy, 2)); 

    float spaceshipRadius = 30.0f;  
    float asteroidRadius = 20.0f; // temp, get real size / 2

    return distance < (spaceshipRadius + asteroidRadius); //circle collision formula
}




void moveAsteroids(ecs::Entity* spaceship) 
{
    if (!spaceship) {
        DINFOF("Spaceship is null\n");
        return; 
    }


    auto asteroids = ecs::ecs.query<ecs::Asteroid, ecs::Transform>(); //query asteroid w/ transform
    for (auto* asteroid: asteroids) { // chat: how loop to through all asteroids. 
       
        auto transform = ecs::ecs.component().fetch<ecs::Transform>(asteroid);
        auto asteroidComp = ecs::ecs.component().fetch<ecs::Asteroid>(asteroid);
        auto sprite = ecs::ecs.component().fetch<ecs::Sprite>(asteroid);

        if (!transform || !asteroidComp || !sprite)
            continue; 
        

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

        
        transform->pos[0] -= asteroidComp->movementSpeed; // move asteroids left

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

                auto shipHealth = ecs::ecs.component().fetch<HEALTH>(spaceship);

                if (shipHealth) {
                    shipHealth->health -= 1.0f; 
                    DINFOF("Spaceship damaged, health is now: %.2f\n", shipHealth->health);
                } else {
                    DINFO("no health comp\n");

                }
            }

        }
    }

}





