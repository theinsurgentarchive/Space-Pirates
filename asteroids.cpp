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
#include <csignal>
#include <thread>
#include <future>

#include "image.h"
#include "log.h"
#include "fonts.h"

#include "jlo.h"
#include "balrowhany.h"
#include "jsandoval.h"
#include "mchitorog.h"
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
	v2u res;
	char keys[65536];
	int mouse_cursor_on;
	GLuint walkTexture;
	GLuint titleTexture;
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
		res[0] = 720;
		res[1] = 480;
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
GLuint planet4Texture;   
Image *menuImage = NULL;
Image *planetImage;
Image *planet2Image;
Image *planet4Image; 

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
		setup_screen_res(gl.res[0],gl.res[1]);
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			std::cout << "\n\tcannot connect to X server" << std::endl;
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XWindowAttributes getWinAttr;
		XGetWindowAttributes(dpy, root, &getWinAttr);
		int fullscreen = 0;
		gl.res[0] = w;
		gl.res[1] = h;
		if (!w && !h) {
			//Go to fullscreen.
			gl.res[0] = getWinAttr.width;
			gl.res[1] = getWinAttr.height;
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
		win = XCreateWindow(dpy, root, 0, 0, gl.res[0], gl.res[1], 0,
				vi->depth, InputOutput, vi->visual, winops, &swa);
		//win = XCreateWindow(dpy, root, 0, 0, gl.res[0], gl.res[1], 0,
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
			if (xce.width != gl.res[0] || xce.height != gl.res[1]) {
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
			glOrtho(0, gl.res[0], 0, gl.res[1], -1, 1);
			gl.res[0] = width;
			gl.res[1] = height;
			set_title();
		}
		void setup_screen_res(const int w, const int h) {
			gl.res[0] = w;
			gl.res[1] = h;
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
	} x11(gl.res[0],gl.res[1]);
		
			// glOrtho(0, gl.res[0], 0, gl.res[1], -1, 1);
// ---> for fullscreen x11(0, 0);

//function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();
// For transparent title.png background
std::unique_ptr<unsigned char[]> buildAlphaData(Image *img);

//Start - Justin
//==========================================================================
// M A I N
//==========================================================================
ecs::Entity* ptr;

ecs::Entity* planetPtr;
ecs::Entity* planetPtr2;
ecs::RenderSystem rs {ecs::ecs,60};
ecs::PhysicsSystem ps {ecs::ecs,5};
const Camera* c;
std::unordered_map<std::string,std::shared_ptr<Texture>> textures;
std::unordered_map<std::string,std::shared_ptr<SpriteSheet>> ssheets;
std::vector<Collision> cols;
atomic<bool> done = false;
void sig_handle(int sig)
{
	done = true;
	std::exit(0);
}
// load space sheets
std::unordered_map<std::string, std::shared_ptr<SpriteSheet>> shipAndAsteroidsSheets;
void loadShipAndAsteroids(std::unordered_map<std::string, std::shared_ptr<SpriteSheet>>& shipAndAsteroidsSheets);
ecs::RenderSystem spaceRenderer {ecs::ecs, 60};
int main()
{
	ThreadPool tp {4};
	std::signal(SIGINT,sig_handle);
	std::signal(SIGTERM,sig_handle);
	gl.spaceship = ecs::ecs.entity().checkout(); 
	initializeEntity(gl.spaceship); 
	DINFOF("spaceship initialized spaceship %s", "");
	planetPtr = ecs::GeneratePlanet();
	planetPtr2 = ecs::GeneratePlanet();

	auto [planetAttr] = ecs::ecs.component().fetch<PLANET>(planetPtr);
	auto [planetAttr2] = ecs::ecs.component().fetch<PLANET>(planetPtr2);

	WorldGenerationSettings settings {
		planetAttr->temperature,
		planetAttr->humidity,
		static_cast<u16>(planetAttr->size * 50),
		static_cast<u32>(2)};
	settings.origin = {0,0};

    // Initialize audio system
    initAudioSystem();
	loadTextures(ssheets);  //load planet textures
	loadShipAndAsteroids(ssheets); // load ship and asteroids
    
    // Set initial music according to game state (starting in MENU state)
    updateAudioState(gl.state);

    // ecs::ecs.component().bulkAssign<PHYSICS,SPRITE,TRANSFORM,HEALTH,NAME,COLLIDER>(ptr);
	ptr = ecs::ecs.entity().checkout();
	auto [transform,sprite,name,collider,health,p] = ecs::ecs.component().assign<TRANSFORM,SPRITE,NAME,COLLIDER, HEALTH,PHYSICS>(ptr);
	Camera camera = {
		transform->pos,
		gl.res
	};
	name->name = "Simon";
	name->offset = {0,-25};
	sprite->ssheet = "player-idle";
	sprite->render_order = 15;
	collider->offset = {0.0f,-8.0f};
	collider->dim = v2u {5,4};
	health->health = 50;
	health->max = 100;
	loadTextures(ssheets);
	c = &camera;

	World w {settings};
	rs.sample();
	ps.sample();
	init_opengl();
	logOpen();
	srand(time(NULL));
	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);
	x11.set_mouse_position(200, 200);
	x11.show_mouse_cursor(gl.mouse_cursor_on);
	tp.enqueue([&camera,&tp]() { collisions(camera,tp); });
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
        getAudioManager()->update();
		physics();
		render();
        x11.swapBuffers();
        usleep(1000);
    }
    shutdownAudioSystem();
    cleanup_fonts();
    logClose();
    return 0;
}
GLuint tex;
void init_opengl(void)
{
	glViewport(0, 0, gl.res[0],gl.res[1]);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_TEXTURE_2D);

	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
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
	glTexImage2D(
			GL_TEXTURE_2D, 0, 3, 
			planetImage->width, planetImage->height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, planetImage->data.get()
		    );

	glGenTextures(1, &planet2Texture);
	planet2Image = new Image("./resources/textures/planet.gif");	
	//biship code 
	glBindTexture(GL_TEXTURE_2D, planet2Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(
			GL_TEXTURE_2D, 0, 3,
			planet2Image->width, planet2Image->height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, planet2Image->data.get()
		    );

	glGenTextures(1, &planet4Texture);
	planet4Image = new Image("./resources/textures/planet4.webp");	
	//biship code 
	glBindTexture(GL_TEXTURE_2D, planet4Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(
			GL_TEXTURE_2D, 0, 3,
			planet4Image->width, planet4Image->height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, planet4Image->data.get()
		    );

	glGenTextures(1, &planet4Texture);
	planet4Image = new Image("./resources/textures/planet4.webp");	
	//biship code 
	glBindTexture(GL_TEXTURE_2D, planet4Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, planet4Image->width, planet4Image->height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, planet4Image->data.get());

	initialize_fonts();
	// Load game title texture
	glGenTextures(1, &gl.titleTexture);
	Image *titleImage = new Image("./resources/textures/title.png");
	glBindTexture(GL_TEXTURE_2D, gl.titleTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Use GL_RGBA instead of GL_RGB to handle alpha channel
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, titleImage->width, titleImage->height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, buildAlphaData(titleImage).get());

}

std::unique_ptr<unsigned char[]> buildAlphaData(Image* img)
{
	auto img_size = img->width * img->height;
	auto newdata = std::make_unique<unsigned char[]>(img_size * 4);
	auto* data = img->data.get();
	auto* ptr = newdata.get();
	auto t0 = data[0], t1 = data[1], t2 = data[2];
	for (int i = 0; i < img_size; ++i) {
		// Copy RGB values
		std::copy(data, data + 3, ptr);

		// This helps with anti-aliased edges
		bool is_background = (data[0] == t0 && data[1] == t1 && data[2] == t2);
		bool is_near_background = (abs(data[0] - t0) < 10 && 
				abs(data[1] - t1) < 10 && 
				abs(data[2] - t2) < 10);

		// Full transparency for background, semi-transparency for edges
		if (is_background)
			ptr[3] = 0;  // Fully transparent
		else if (is_near_background)
			ptr[3] = 128; // Semi-transparent for better edge blending
		else
			ptr[3] = 255; // Fully opaque

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
	[[maybe_unused]]static int exit_request = 0;
	if (e->type != KeyRelease && e->type != KeyPress) {
		//not a keyboard event
		return 0;
	}

	// Not a keyboard event
	if (e->type != KeyRelease && e->type != KeyPress) {
		return exit_request;
	}
	int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);

	if (e->type == KeyPress) {
		exit_request = handle_menu_keys(key, gl.state, gl.selected_option);
		if (exit_request) {
			return exit_request;
		}
		// Add handling for ESC key in PLAYING state
		if (key == XK_Escape && gl.state == PLAYING) {
			// Pause game music
			getAudioManager()->pauseMusic();
			// Play menu music
			playGameMusic(MENU_MUSIC);
			// Change state to MENU
			gl.state = MENU;
			return 0;
		}

		if (key == XK_Escape && gl.state == SPACE) {  // TEMP FOR EASE
			gl.state = MENU; 
			return 0;
		}

		if (key == XK_Escape && gl.state == CREDITS) {
			gl.state = MENU; 
			return 0;
		}
	}

	// Playing state handling
	if (gl.state == PLAYING) {
		if (!ecs::ecs.component().has<PHYSICS>(ptr)) {
			return 0;
		}
		auto [pc,sc] = ecs::ecs.component().fetch<PHYSICS,SPRITE>(ptr);
		if (e->type == KeyRelease) {
			if (key == XK_Up || key == XK_Down || key == XK_Left || key == XK_Right) {
				sc->ssheet = "player-idle";
				sc->invert_y = false;
				pc->vel = {0,0};
			}
		} else if (e->type == KeyPress) {
			static float movement_mag = 300.0;
			switch(key) {
				case XK_Right:
					sc->ssheet = "player-right";
					pc->vel = {movement_mag,0};
					break;
				case XK_Left:
					sc->invert_y = true;
					sc->ssheet = "player-right";
					pc->vel = {-movement_mag,0};
					break;
				case XK_Up:
					sc->ssheet = "player-back";
					pc->vel = {0,movement_mag};
					break;
				case XK_Down:
					sc->ssheet = "player-front";
					pc->vel = {0,-movement_mag};
					break;
				case XK_a:
					done = 1;
					break;
			}
		}
	}

	if (gl.state == SPACE) {
		[[maybe_unused]] auto [transform,sprite,physics] = ecs::ecs.component().fetch<TRANSFORM,SPRITE,PHYSICS>(gl.spaceship);
		if (e->type == KeyPress) {
			static float movement_mag = 300.0;
			switch(key) {
				case XK_Right:
					sprite->ssheet = "ship-right";
					physics->vel = {movement_mag,0};
					break;
				case XK_Left:
					sprite->invert_y = true;
					sprite->ssheet = "ship-right";
					physics->vel = {-movement_mag,0};
					break;
				case XK_Up:
					sprite->ssheet = "ship-front-back";
					physics->vel = {0,movement_mag};
					break;
				case XK_Down:
					sprite->ssheet = "ship-front-back";
					physics->vel = {0,-movement_mag};
					break;
				case XK_a:
					done = 1;
					break;
				}

			} else if (e->type == KeyRelease) {
				sprite->ssheet = "ship-right"; 
				sprite ->invert_y = false;
				physics->vel = {0,0};
			}

		} return exit_request;
	} //bracket fix



void physics()
{
	ps.update((float)1/20);
	if (gl.state == MENU){
		gl.planetAng[2] += 1.0;
	}
	else if (gl.state == SPACE) {
		auto [traits] = ecs::ecs.component().fetch<PLANET>(planetPtr);
		traits-> AngY += 1.0f;
		// ecs::updatePlanetSpin();
	}
}

 void SampleSpaceEntities() //chatgpt 
 { // sample space entities, made jlo renderSystem > ._entities public for access
 	auto spaceEntities = ecs::ecs.query<ASTEROID>(); 
	if (gl.spaceship) {
		spaceEntities.push_back(gl.spaceship); // add spaceship to entity list
	}
	spaceRenderer._entities = spaceEntities; //render only these (filtered)
 }


void render() {
	DINFOF("rendering state: %d\n", gl.state);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	Rect r;
	r.left = 100;
	r.bot = gl.res[1] - 20;
	auto [tc] = ecs::ecs.component().fetch<TRANSFORM>(ptr);
	auto [traits] = ecs::ecs.component().fetch<PLANET>(planetPtr);
	float cameraX = static_cast<float>(tc->pos[0]);
	float cameraY = static_cast<float>(tc->pos[1]);
	switch(gl.state) {
		case MENU:
			// Setup for 2D rendering (menu interface)
			glMatrixMode(GL_PROJECTION);
			glPushMatrix(); // PUSH 1
			glLoadIdentity();
			glOrtho(0, gl.res[0], 0, gl.res[1], -1, 1);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix(); // PUSH 2
			glLoadIdentity();

			render_menu_screen(gl.res[0], gl.res[1], menuBackgroundTexture, gl.titleTexture, gl.selected_option); 

			glPopMatrix(); // POP 2
			glMatrixMode(GL_PROJECTION);
			glPopMatrix(); // POP 1

			// Setup for 3D rendering (planets)
			glMatrixMode(GL_PROJECTION);
			glPushMatrix(); // PUSH 3
			glLoadIdentity();
			gluPerspective(45.0f, (GLfloat) gl.res[0] / (GLfloat) gl.res[1], 0.1f, 100.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix(); // PUSH 4
			glLoadIdentity();
			gluLookAt(0.0f, 5.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

			// Draw planets
			glPushMatrix(); // PUSH 5
			DrawPlanetMenu(
					gl.planetAng[2], gl.planetPos[0] - 4.5, gl.planetPos[1] - 3,
					gl.planetPos[2], gl.lightPosition, planet2Texture, 3, 1, 1
				  );
			glPopMatrix(); // POP 5

			glPushMatrix(); // PUSH 6
			DrawPlanetMenu(
					gl.planetAng[2], gl.planetPos[0] + 5.5, gl.planetPos[1],
					gl.planetPos[2], gl.lightPosition, planetTexture, 2.25, 1, 0
				  );
			glPopMatrix(); // POP 6

			glPushMatrix(); // PUSH 7
			DrawPlanetMenu(
					gl.planetAng[2], gl.planetPos[0] + 1.4, gl.planetPos[1] - 7,
					gl.planetPos[2], gl.lightPosition, planet4Texture, 1, 0, 1
				  );
			glPopMatrix(); // POP 7

			glPopMatrix(); // POP 4
			glMatrixMode(GL_PROJECTION);
			glPopMatrix(); // POP 3
			break;

		case CONTROLS:
			// Reset GL state completely for controls screen
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, gl.res[0], 0, gl.res[1], -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			render_control_screen(gl.res[0], gl.res[1], menuBackgroundTexture);
			break;

		case CREDITS:
			// Reset GL state completely for credits screen
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, gl.res[0], 0, gl.res[1], -1, 1);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			render_credits_screen(gl.res[0], gl.res[1], menuBackgroundTexture);
			break;

		case PLAYING:
			// Reset for game state
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, gl.res[0], 0, gl.res[1], -1, 1);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glPushMatrix();
			c->update();
			rs.update((float)1/10);
			glPopMatrix();

			DisableFor2D();
			if (ptr) {   //player health bar
				auto [playerHealth] = ecs::ecs.component().fetch<ecs::Health>(ptr);
				playerHealth->health = 100.0f; 
				playerHealth -> max = 100.0f;
				if (playerHealth) 
					drawUIBar("Health", playerHealth->health, playerHealth->max, 20, gl.res[1] - 50, 0xF00FF00);
			}

			ggprint8b(&r, 0, 0xffffffff, "position: %f %f", cameraX, cameraY);
			break; 

		case SPACE:
		{
			glMatrixMode(GL_PROJECTION);
			glPushMatrix(); // PUSH 3
			glLoadIdentity();
			gluPerspective(45.0f, (GLfloat)gl.res[0] / (GLfloat)gl.res[1], 0.1f, 100.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix(); // PUSH 4
			glLoadIdentity();
			gluLookAt(0.0f, 5.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
			EnableFor3D();
			glPushMatrix();
			DrawPlanet(traits-> AngY, traits-> PosX, traits-> PosY, traits-> 
				PosZ, gl.lightPosition, traits->size, traits->rotationX, 
				traits->rotationY, traits->smooth, traits->temperature);
			glPopMatrix();

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, gl.res[0], 0, gl.res[1], -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			if (gl.spaceship) { //draw ui space bars
				auto [spaceshipHealth,oxygen,fuel] = ecs::ecs.component().fetch<ecs::Health,ecs::Oxygen,ecs::Fuel>(gl.spaceship);			
				if (spaceshipHealth) {
					drawUIBar("Health", spaceshipHealth->health, spaceshipHealth->max, 20, gl.res[1] - 50, 0xF00FF00);
				}
				if (oxygen) {
					drawUIBar("Oxygen", oxygen->oxygen, oxygen->max, 20, gl.res[1] - 90, 0x00FFFF);
				}
				if (fuel) {
					drawUIBar("Fuel", fuel->fuel, fuel->max, 20, gl.res[1] - 130, 0xFF9900);
				}
			}
			// merge into ssheets global in space mode
			// static bool spaceSheetsLoaded = false;
			// if (!spaceSheetsLoaded) {
    		// 	ssheets.insert(shipAndAsteroidsSheets.begin(), shipAndAsteroidsSheets.end());
   			// 	spaceSheetsLoaded = true; 
				
			// }
		
			spawnAsteroids(gl.spaceship, gl.res[0], gl.res[1]); 
			c->update(); // update camera
			//spaceRenderer.sample(); // sample space entities
			SampleSpaceEntities(); //chat: update entity list w/ asteroids
			spaceRenderer.update((float)1/10); // update space render system
			//cout << "SpaceRenderer updated" << endl;

	
			DisableFor2D();
			ggprint8b(&r, 0, 0xFFFFFF, "Welcome to SPACE mode 🚀");
			break;
			
		}

		case EXIT: 
			break; 


		default: 
			break; 

	}

}
			