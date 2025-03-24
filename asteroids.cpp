//
//program: asteroids.cpp
//author:  Gordon Griesel
//date:    2014 - 2025
//mod spring 2015: added constructors
//This program is a game starting point for a 3350 project.
//
//
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#include "image.h"
#include "log.h"
#include "fonts.h"

#include "jlo.h"
#include "balrowhany.h"
#include "jsandoval.h"
#define GAME_TITLE "Space Pirates"
#define _TEXTURES "./textures"
using namespace std; 

// #define CREDITS
//defined types
typedef float Flt;
typedef float Vec[3];
typedef Flt	Matrix[4][4];
//constants
uint16_t counter = 0;
const float timeslice = 1.0f;
const float gravity = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
#define rnd() (float)rand() / (float)RAND_MAX
const int MAX_BULLETS = 11;
const Flt MINIMUM_ASTEROID_SIZE = 60.0;
//-----------------------------------------------------------------------------
//Setup timers
const double physicsRate = 1.0 / 60.0;
const double oobillion = 1.0 / 1e9;
extern struct timespec timeStart, timeCurrent;
extern struct timespec timePause;
extern double physicsCountdown;
extern double timeSpan;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
//std::unique_ptr<unsigned char[]> buildAlphaData(Image *img);
//-----------------------------------------------------------------------------

class Global {
public:
	int xres, yres;
	char keys[65536];
	int mouse_cursor_on;
	GLuint walkTexture;
	GLfloat lightAmbient[4];
	GLfloat lightDiffuse[4];
	GLfloat lightSpecular[4];
	GLfloat lightPosition[4];
	float planetPos[3];
	float planetRot[3];
	float planetAng[3];



	GameState state; 
	int selected_option; // 0 = start, 1 = controls, 2 = exit
	ecs::Entity* spaceship;

	Global() {
		xres = 1280;
		yres = 960;
		memset(keys, 0, 65536);
		// mouse value 1 = true = mouse is a regular mouse.
		state = MENU; // default 
		mouse_cursor_on = 1;

		//planet shadow
		//GLfloat la[]  = {  0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat ld[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat ls[] = {  0.5f, 0.5f, 0.5f, 1.0f };

		GLfloat lp[] = { 100.0f, 60.0f, -140.0f, 1.0f };
		lp[0] = rnd() * 200.0 - 100.0;
		lp[1] = rnd() * 100.0 + 20.0;
		lp[2] = rnd() * 300.0 - 150.0;
		memcpy(lightPosition, lp, sizeof(GLfloat)*4);
		memcpy(lightDiffuse, ld, sizeof(GLfloat)*4);
		memcpy(lightSpecular, ls, sizeof(GLfloat)*4);
		memcpy(lightPosition, lp, sizeof(GLfloat)*4);

		float bp[3]={0.0,2.0,-7.0};
		float ba[3]={0.0,0.0,0.0};
		memcpy(planetPos, bp, sizeof(float)*3);
		memcpy(planetRot, ba, sizeof(float)*3);
		memcpy(planetAng, ba, sizeof(float)*3);
	}
}; 
Global gl;
GLuint menuBackgroundTexture;
GLuint planetTexture;
GLuint planet2Texture;   
Image *menuImage = NULL;
Image *planetImage;
Image *planet2Image; 

class Game {
public:
	int nasteroids;
	int nbullets;
	struct timespec bulletTimer;
	struct timespec mouseThrustTimer;
	bool mouseThrustOn;
public:
	Game() {
		nasteroids = 0;
		nbullets = 0;
		mouseThrustOn = false;
		
		clock_gettime(CLOCK_REALTIME, &bulletTimer);
	}
} g;

//X Windows variables
class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	X11_wrapper() { }
	X11_wrapper(int w, int h) {
		GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
		XSetWindowAttributes swa;
		setup_screen_res(gl.xres, gl.yres);
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			std::cout << "\n\tcannot connect to X server" << std::endl;
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XWindowAttributes getWinAttr;
		XGetWindowAttributes(dpy, root, &getWinAttr);
		int fullscreen = 0;
		gl.xres = w;
		gl.yres = h;
		if (!w && !h) {
			//Go to fullscreen.
			gl.xres = getWinAttr.width;
			gl.yres = getWinAttr.height;
			//When window is fullscreen, there is no client window
			//so keystrokes are linked to the root window.
			XGrabKeyboard(dpy, root, False,
				GrabModeAsync, GrabModeAsync, CurrentTime);
			fullscreen=1;
		}
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			std::cout << "\n\tno appropriate visual found\n" << std::endl;
			exit(EXIT_FAILURE);
		} 
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		swa.colormap = cmap;
		swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | MotionNotify | ButtonPress | ButtonRelease |
			StructureNotifyMask | SubstructureNotifyMask;
		unsigned int winops = CWBorderPixel|CWColormap|CWEventMask;
		if (fullscreen) {
			winops |= CWOverrideRedirect;
			swa.override_redirect = True;
		}
		win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
			vi->depth, InputOutput, vi->visual, winops, &swa);
		//win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
		//vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
		show_mouse_cursor(0);
	}
	~X11_wrapper() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void set_title() {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, GAME_TITLE);
	}
	void check_resize(XEvent *e) {
		//The ConfigureNotify is sent by the
		//server if the window is resized.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		if (xce.width != gl.xres || xce.height != gl.yres) {
			//Window size did change.
			reshape_window(xce.width, xce.height);
		}
	}
	void reshape_window(int width, int height) {
		//window has been resized.
		setup_screen_res(width, height);
		glViewport(0, 0, (GLint)width, (GLint)height);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
		glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
		set_title();
	}
	void setup_screen_res(const int w, const int h) {
		gl.xres = w;
		gl.yres = h;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}
	bool getXPending() {
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void set_mouse_position(int x, int y) {
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
	}
	void show_mouse_cursor(const int onoff) {
		printf("show_mouse_cursor(%i)\n", onoff); fflush(stdout);
		if (onoff) {
			//this removes our own blank cursor.
			XUndefineCursor(dpy, win);
			return;
		}
		//vars to make blank cursor
		Pixmap blank;
		XColor dummy;
		char data[1] = {0};
		Cursor cursor;
		//make a blank cursor
		blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
		if (blank == None)
			std::cout << "error: out of memory." << std::endl;
		cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
		XFreePixmap(dpy, blank);
		//this makes the cursor. then set it using this function
		XDefineCursor(dpy, win, cursor);
		//after you do not need the cursor anymore use this function.
		//it will undo the last change done by XDefineCursor
		//(thus do only use ONCE XDefineCursor and then XUndefineCursor):
	}
} x11(gl.xres, gl.yres);
// ---> for fullscreen x11(0, 0);

//function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

//Start - Justin
//==========================================================================
// M A I N
//==========================================================================
ecs::Entity* ptr;
ecs::RenderSystem rs {ecs::ecs,60};
ecs::PhysicsSystem ps {ecs::ecs,5};
const World* world;
const Camera* c;
int done;
std::unordered_map<std::string,std::shared_ptr<Texture>> textures;
std::unordered_map<std::string,std::shared_ptr<SpriteSheet>> ssheets;
int main()
{
	gl.spaceship = ecs::ecs.entity().checkout(); 
	initializeEntity(gl.spaceship); 
	DINFOF("spaceship initialized spaceship %s", "");

    // [[maybe_unused]]auto character = ecs::character_x(); 

    // Initialize audio system
	auto biome = selectBiome(30.0f,0.5f);
	std::cout << biome.type << ' ' << biome.description << '\n';
    initAudioSystem();
    
    // Set initial music according to game state (starting in MENU state)
    updateAudioState(gl.state);

    ptr = ecs::ecs.entity().checkout();
    ecs::ecs.component().bulkAssign<PHYSICS,SPRITE,TRANSFORM,HEALTH>(ptr);
	
	auto tc = ecs::ecs.component().fetch<TRANSFORM>(ptr);
	Camera camera {tc->pos,{gl.xres,gl.yres}};
	c = &camera;
	auto sc = ecs::ecs.component().fetch<SPRITE>(ptr);
	sc->ssheet = "skip";
	sc->render_order = 15;
	ssheets.insert({"skip",
		std::make_shared<SpriteSheet>(
			v2u {1,8},
			v2u {32,32},
			loadTexture("./resources/textures/skip.png", true)
		)
	});

	ssheets.insert({"sand",
		std::make_shared<SpriteSheet>(
			v2u {1,1},
			v2u {16,16},
			loadTexture("./resources/textures/sand.webp",false)
		)
	});

	ssheets.insert({"water",
		std::make_shared<SpriteSheet>(
			v2u {1,1},
			v2u {16,16},
			loadTexture("./resources/textures/water.webp",false)
		)
	});

	ssheets.insert({"grass",
		std::make_shared<SpriteSheet>(
			v2u {1,1},
			v2u {16,16},
			loadTexture("./resources/textures/grass.webp",false)
		)
	});

	Vec2<uint16_t> v {50,50};
	std::unordered_map<std::string,wfc::TileMeta> tile_map;
    tile_map.insert({"A",wfc::TileBuilder{0.6,"grass"}.omni("A").omni("C").coefficient("A",3).coefficient("_",-0.2).build()});
    tile_map.insert({"_",wfc::TileBuilder{0.6,"water"}.omni("C").omni("_").coefficient("_",5).build()});
	tile_map.insert({"C",wfc::TileBuilder{0.3,"sand"}.omni("_").coefficient("C",3).omni("C").omni("A").build()});
	std::unordered_set<std::string> tiles;
	for (auto& pair : tile_map) {
		tiles.insert(pair.first);
	}
	wfc::Grid grid {v, tiles};
	wfc::WaveFunction wf {grid,tile_map};
	wf.run();
	auto w = World{{0,0},grid,tile_map};
	world = &w;
	rs.sample();
	ps.sample();
	init_opengl();
	logOpen();
	srand(time(NULL));
	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);
	x11.set_mouse_position(200, 200);
	x11.show_mouse_cursor(gl.mouse_cursor_on);
	done = 0;
    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
		}
        clock_gettime(CLOCK_REALTIME, &timeCurrent);
        timeSpan = timeDiff(&timeStart, &timeCurrent);
        timeCopy(&timeStart, &timeCurrent);

		auto current = std::chrono::high_resolution_clock::now();
		// auto dur = std::chrono::duration_cast<std::chrono::seconds>(current - rs.lastSampled());
		// if (dur.count() >= rs.sample_delta) {
		// 	rs.sample();
		// }
		auto dur = std::chrono::duration_cast<std::chrono::seconds>(current - ps.lastSampled());
		if (dur.count() >= ps.sample_delta) {
			ps.sample();
		}
		//clear screen just once at the beginning
        glClear(GL_COLOR_BUFFER_BIT); 

        // Update audio system each frame
        getAudioManager()->update();
        ps.update((float) 1/20);
        render();
		physics(); 
        x11.swapBuffers();
        usleep(10000);
    }
    shutdownAudioSystem();
    cleanup_fonts();
    logClose();
    return 0;
}
GLuint tex;
void init_opengl(void)
{
	glViewport(0, 0, gl.xres, gl.yres);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	//wen enabled, buttons arent shown (glEnable)
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	
	// glMatrixMode(GL_PROJECTION);
	
			// glLoadIdentity();
	//JC when gluPerspective enable screen breaks for menu 
	//gluPerspective(45.0f,(GLfloat)gl.xres/(GLfloat)gl.yres,0.1f,100.0f);
	// glMatrixMode(GL_MODELVIEW);
					//glLoadIdentity();
					//gluLookAt(0,5,10,  0,0,0,  0,1,0);
					//Enable this so material colors are the same as vert colors.
	glEnable(GL_COLOR_MATERIAL);
	
	glEnable( GL_LIGHTING );
	glLightfv(GL_LIGHT0, GL_AMBIENT, gl.lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, gl.lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, gl.lightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, gl.lightPosition);
	glEnable(GL_LIGHT0);

	glGenTextures(1, &menuBackgroundTexture);
	menuImage = new Image("./resources/textures/menu-bg.webp");	
	//biship code 
	glBindTexture(GL_TEXTURE_2D, menuBackgroundTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, menuImage->width, menuImage->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, menuImage->data.get());

	glGenTextures(1, &planetTexture);
	planetImage = new Image("./resources/textures/planet2.gif");	
	//biship code 
	glBindTexture(GL_TEXTURE_2D, planetTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, planetImage->width, planetImage->height, 0,
				GL_RGB, GL_UNSIGNED_BYTE, planetImage->data.get());

	glGenTextures(1, &planet2Texture);
	planet2Image = new Image("./resources/textures/planet.gif");	
	//biship code 
	glBindTexture(GL_TEXTURE_2D, planet2Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, planet2Image->width, planet2Image->height, 0,
				GL_RGB, GL_UNSIGNED_BYTE, planet2Image->data.get());

	initialize_fonts();


}

std::unique_ptr<unsigned char[]> buildAlphaData(Image* img)
{
	//refactored version of gordon's code
    auto img_size = img->width * img->height;
    auto newdata = std::make_unique<unsigned char[]>(img_size * 4);
    auto* data = img->data.get();
    auto* ptr = newdata.get();
    auto t0 = data[0], t1 = data[1], t2 = data[2];
    for (int i = 0; i < img_size; ++i) {
        std::copy(data, data + 3, ptr);
        ptr[3] = (data[0] == t0 && data[1] == t1 && data[2] == t2) ? 0 : 255;
        data += 3;
        ptr += 4;
    }
    return newdata;
}

void normalize2d(Vec v)
{
	Flt len = v[0]*v[0] + v[1]*v[1];
	if (len == 0.0f) {
		v[0] = 1.0;
		v[1] = 0.0;
		return;
	}
	len = 1.0f / sqrt(len);
	v[0] *= len;
	v[1] *= len;
}

void check_mouse(XEvent *e)
{
	//Did the mouse move?
	//Was a mouse button clicked?
	[[maybe_unused]] static int savex = 0;
	[[maybe_unused]] static int savey = 0;
	//
	[[maybe_unused]] static int ct=0;
	//std::cout << "m" << std::endl << std::flush;
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		}
		if (e->xbutton.button==3) {
			//Right button is down
		}
	//keys[XK_Up] = 0;
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		// the mouse moved
		[[maybe_unused]] int xdiff = savex - e->xbutton.x;
		[[maybe_unused]] int ydiff = savey - e->xbutton.y;
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++ct < 10)
			return;		
		// If mouse cursor is on, it does not control the ship.
		// It's a regular mouse.
		if (gl.mouse_cursor_on)
			return;
	}
}

int check_keys(XEvent *e)
{
    [[maybe_unused]]static int shift = 0;
    [[maybe_unused]]static int exit_request = 0;  // Initialize to 0
	if (e->type != KeyRelease && e->type != KeyPress) {
		//not a keyboard event
		return 0;
	}
	// if (!ecs::ecs.component().has<ecs::Physics>(ptr)) {
	// 	return 0;
	// }

    // Not a keyboard event
    if (e->type != KeyRelease && e->type != KeyPress) {
        return exit_request;
    }

    int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);

    // handle key events - modified to add menu state handling, 
    // cout for debugging
    if (e->type == KeyPress) {
      exit_request = handle_menu_keys(key, gl.state, gl.selected_option);
	  	if (exit_request) {
			return exit_request;
    	}
	}

    // Playing state handling
    if (gl.state == PLAYING) {
        if (!ecs::ecs.component().has<PHYSICS>(ptr)) {
            return 0;
        }
        auto pc = ecs::ecs.component().fetch<ecs::Physics>(ptr);
        if (e->type == KeyRelease) {
            if (key == XK_Up || key == XK_Down || key == XK_Left || key == XK_Right) {
                pc->vel = {0,0};
            }
        } else if (e->type == KeyPress) {
			
            static float movement_mag = 300.0;
            switch(key) {
                case XK_Right:
                    pc->vel = {movement_mag,0};
                    break;
                case XK_Left:
                    pc->vel = {-movement_mag,0};
                    break;
                case XK_Up:
                    pc->vel = {0,movement_mag};
                    break;
                case XK_Down:
                    pc->vel = {0,-movement_mag};
                    break;
				case XK_a:
					done = 1;
					break;
            }
        }
    }

    return exit_request;
}

void physics()
{
	ps.update(1/20);
	gl.planetAng[2] += 1.0;

}

void render() {
	//DrawPlanet(gl.planetAng[2], gl.planetPos[0], gl.planetPos[1], gl.planetPos[2], gl.lightPosition, planetTexture);
	DINFOF("rendering state: %d\n",gl.state);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	Rect r;
	r.left = 100;
	r.bot = gl.yres - 20;
	auto tc = ecs::ecs.component().fetch<TRANSFORM>(ptr);
	float cameraX = static_cast<float>(tc->pos[0]);
	float cameraY = static_cast<float>(tc->pos[1]);

	switch(gl.state) {
		case MENU:
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, gl.xres, 0, gl.yres, -1, 1);

			glMatrixMode(GL_MODELVIEW); 
			glPushMatrix(); glLoadIdentity();

			render_menu_screen(gl.xres, gl.yres, menuBackgroundTexture, gl.selected_option);
			
			glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluPerspective(45.0f, (GLfloat)gl.xres / (GLfloat)gl.yres, 0.1f, 100.0f);
			
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			gluLookAt(0.0f, 5.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

			glPushMatrix();
			DrawPlanet(gl.planetAng[2], gl.planetPos[0]-4.5, gl.planetPos[1]-3, gl.planetPos[2], gl.lightPosition, planet2Texture, 3, 1, 1);
			glPopMatrix();

			glPushMatrix();
			DrawPlanet(gl.planetAng[2], gl.planetPos[0]+5.5, gl.planetPos[1], gl.planetPos[2], gl.lightPosition, planetTexture, 2.25, 0, 1);
			glPopMatrix();

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			break;
		case CONTROLS:
			render_control_screen(gl.xres, gl.yres, menuBackgroundTexture);
			break;
		case PLAYING:
			glPushMatrix();
			c->update();
			rs.update((float)1/10);
			glPopMatrix();
			
			DisableFor2D();
			ggprint8b(&r,0,0xffffffff,"position: %f %f",cameraX,cameraY);
			
			//bar
			if (gl.spaceship) {
				auto spaceshipHealth = ecs::ecs.component().fetch<ecs::Health>(gl.spaceship);
				auto spaceshipOxygen = ecs::ecs.component().fetch<ecs::Oxygen>(gl.spaceship);
				auto spaceshipFuel = ecs::ecs.component().fetch<ecs::Fuel>(gl.spaceship);
			
				if (spaceshipHealth) {
					drawUIBar("Health", spaceshipHealth->health, spaceshipHealth->max, 20, gl.yres - 50, 0xF00FF00);
				}
			
				if (spaceshipOxygen) {
					drawUIBar("Oxygen", spaceshipOxygen->oxygen, spaceshipOxygen->max, 20, gl.yres - 90, 0x00FFFF);
				}
			
				if (spaceshipFuel) {
					drawUIBar("Fuel", spaceshipFuel->fuel, spaceshipFuel->max, 20, gl.yres - 130, 0xFF9900);
				}
			}
			break;
		case EXIT:
			break;
		default:
			break;
	}
}
