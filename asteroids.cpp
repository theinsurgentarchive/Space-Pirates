//
//program: asteroids.cpp
//author:  Gordon Griesel
//date:    2014 - 2025
//mod spring 2015: added constructors
//This program is a game starting point for a 3350 project.
//
//
#include "jlo.h"
#include "balrowhany.h"
#include "image.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "jsandoval.h"
#define GAME_TITLE "Space Pirates"
#define TEXTURE_FOLDER_PATH "./textures"
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
//-----------------------------------------------------------------------------

class Global {
public:
	int xres, yres;
	char keys[65536];
	int mouse_cursor_on;
	GLuint walkTexture;

	GameState state; 
	int selected_option; // 0 = start, 1 = controls, 2 = exit

	Global() {
		xres = 1280;
		yres = 960;
		memset(keys, 0, 65536);
		// mouse value 1 = true = mouse is a regular mouse.
		state = MENU; // default 
		mouse_cursor_on = 1;
	}
}; 
Global gl;
GLuint menuBackgroundTexture; 
Image *menuImage = NULL; 

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
void load_textures(void);

// void load_textures() {
// 	try {
// 		if (std::filesystem::exists(_IMAGE_FOLDER_PATH) && std::filesystem::is_directory(_IMAGE_FOLDER_PATH)) {
// 			for (const auto& entry : std::file_system::directory_iteraotr<)
// 		}
// 	} catch ()
// }
//==========================================================================
// M A I N
//==========================================================================
ecs::Entity* ptr;
// Entity* second;
// std::unordered_map<std::string,std::shared_ptr<Texture>> textures;
// std::unordered_map<std::string,std::unique_ptr<Animation>> animations;
int main()
{
    auto character = ecs::character_x(); 

	auto e = ecs::ecs.entity().checkout();
	[[maybe_unused]]auto transform = (
		ecs::ecs.component().assign<ecs::Transform>(e)
	);

	//Assign Physics component to entity
	[[maybe_unused]]auto physics = (
		ecs::ecs.component().assign<ecs::Physics>(e)
	);

	//Assign Sprite component to entity
	[[maybe_unused]]auto sprite = (
		ecs::ecs.component().assign<ecs::Sprite>(e)
	);

	ecs::ecs.query<PHYSICS>();
	logOpen();
	init_opengl();
	srand(time(NULL));
	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);
	x11.set_mouse_position(200, 200);
	x11.show_mouse_cursor(gl.mouse_cursor_on);
	int done=0;

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
		//clear screen just once at the beginning
		//glClear(GL_COLOR_BUFFER_BIT); 

		// render based on game state 

		if (gl.state == MENU || gl.state == CONTROLS) {
			//USE RENDER() NOT ECS RENDER SYSTEM IF IN MENU STATE
			render(); 
		} else if (gl.state == PLAYING) {
			//entity_system_manager.update(s, (float) 0.05);
		}
		x11.swapBuffers();
	}
	cleanup_fonts();
	logClose();
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, gl.xres, gl.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//This sets 2D mode (no perspective)
	glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
	//
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	//
	//Clear the screen to black
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();

	glGenTextures(1, &menuBackgroundTexture);
	menuImage = new Image("./textures/menu-bg.jpg");	
	//biship code 
	glBindTexture(GL_TEXTURE_2D, menuBackgroundTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, menuImage->width, menuImage->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, menuImage->data);


	//

}
unsigned char *buildAlphaData(Image *img)
{
	//add 4th component to RGB stream...
	int i;
	unsigned char *newdata, *ptr;
	unsigned char *data = (unsigned char *)img->data;
	//2 bytes * 2 bytes * 4;
	newdata = (unsigned char *)malloc(img->width * img->height * sizeof(int));
	ptr = newdata;
	unsigned char a,b,c;
	//use the first pixel in the image as the transparent color.
	unsigned char t0 = *(data+0);
	unsigned char t1 = *(data+1);
	unsigned char t2 = *(data+2);
	for (i=0; i<img->width * img->height * 3; i+=3) {
		a = *(data+0); //255
		b = *(data+1); //0
		c = *(data+2); //0
		*(ptr+0) = a; //255
		*(ptr+1) = b; //0
		*(ptr+2) = c; //0
		*(ptr+3) = 1; //1
		if (a==t0 && b==t1 && c==t2)
			*(ptr+3) = 0;
		//-----------------------------------------------
		ptr += sizeof(int);
		data += 3;
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

    // handle key events - modified to add menu state handling, cout for debugging
    if (e->type == KeyPress) {
      exit_request = handle_menu_keys(key, gl.state, gl.selected_option);
	  if (exit_request){
		return exit_request;
    }
}

    // Playing state handling
    if (gl.state == PLAYING) {
        if (!ecs::ecs.component().has<ecs::Physics>(ptr)) {
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
            }
        }
    }

    return exit_request;
}

void physics()
{
	
}
void render() {

	DPRINTF("rendering state: %d\n",gl.state);

	glClear(GL_COLOR_BUFFER_BIT);
	// glLoadIdentity(); 
	// glEnable(GL_TEXTURE_2D);  
    // glDisable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    if (gl.state == MENU) {
        //  menu background
		render_menu_screen(gl.xres, gl.yres, menuBackgroundTexture, gl.selected_option);
        }
    else if (gl.state == CONTROLS) {
       render_control_screen(gl.xres, gl.yres, menuBackgroundTexture);
    }
}