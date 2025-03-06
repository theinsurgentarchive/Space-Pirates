//
//program: asteroids.cpp
//author:  Gordon Griesel
//date:    2014 - 2025
//mod spring 2015: added constructors
//This program is a game starting point for a 3350 project.
//
//
#include "jlo.h"
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
#define GAME_TITLE "Space Pirates"
#define DEBUG
#define _TEXTURE_FOLDER_PATH "./textures"
// #define CREDITS
//defined types
typedef float Flt;
typedef float Vec[3];
typedef Flt	Matrix[4][4];
uint16_t counter = 0;
//constants
const float timeslice = 1.0f;
const float gravity = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
const int MAX_BULLETS = 11;
const Flt MINIMUM_ASTEROID_SIZE = 60.0;
Scene s {50};

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
	Global() {
		xres = 1280;
		yres = 960;
		memset(keys, 0, 65536);
		// mouse value 1 = true = mouse is a regular mouse.
		mouse_cursor_on = 1;
	}
} gl;


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
Entity* ptr;
std::unordered_map<std::string,std::shared_ptr<Texture>> textures;
int main()
{
<<<<<<< Updated upstream
	
	TextureLoader loader { _TEXTURE_FOLDER_PATH };
	loader.load_textures(textures);

	
	EntitySystemManager entity_system_manager;
	std::weak_ptr<PhysicsSystem> ps = entity_system_manager.registerSystem<PhysicsSystem>();
	std::weak_ptr<RenderSystem> render_system = entity_system_manager.registerSystem<RenderSystem>();
	ptr = s.createEntity();
	Sprite* sc = s.addComponent<Sprite>(ptr);
	AnimationBuilder ab ("skip.png",{64,64},0);
	sc->animations.insert({"idle",std::make_shared<Animation>(ab.addFrame(0,0).build())});
	sc->animations.insert({"running",std::make_shared<Animation>(ab.addFrame(0,0).addFrame(1,0).addFrame(2,0).addFrame(3,0).addFrame(4,0).addFrame(5,0).addFrame(6,0).addFrame(7,0).build())});
	sc->c_anim = "running";
 	Transform* tc = s.addComponent<Transform>(ptr);
	Physics* pc = s.addComponent<Physics>(ptr);
=======
	auto e = ecs::ecs.entity().checkout();
	while (e != nullptr) {
		//Assign Transform component to entity
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
		
		//Save Changes to entity
		e = ecs::ecs.entity().checkout();
	}
>>>>>>> Stashed changes
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
		// Sprite* sc = s.getComponent<Sprite>(ptr);
		x11.swapBuffers();
		entity_system_manager.update(s, (float) 0.05);
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
	static int shift=0;
	if (e->type != KeyRelease && e->type != KeyPress) {
		//not a keyboard event
		return 0;
	}
	if (!s.hasComponents<Physics>(ptr)) {
		return 0;
	}
	Physics* pc = s.getComponent<Physics>(ptr);
	int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
	if (e->type == KeyRelease) {
		//gl.keys[key] = 0;
		if (key == XK_Up || key == XK_Down || key == XK_Left || key == XK_Right) {
			pc->velocity = {0,0};
		}
		return 0;
	}
	if (e->type == KeyPress) {
		//gl.keys[key]=1;
		static float movement_mag = 300.0;
		switch(key) {
			case XK_Right:
				pc->velocity = {movement_mag,0};
				break;
			case XK_Left:
				pc->velocity = {-movement_mag,0};
				break;
			case XK_Up:
				pc->velocity = {0,movement_mag};
				break;
			case XK_Down:
				pc->velocity = {0,-movement_mag};
				break;
		}
		if (key == XK_Shift_L || key == XK_Shift_R) {

			return 0;
		}
	}
	(void)shift;
	// switch (key) {
	// 	case XK_Escape:
	// 		return 1;
	// 	case XK_m:
	// 		gl.mouse_cursor_on = !gl.mouse_cursor_on;
	// 		x11.show_mouse_cursor(gl.mouse_cursor_on);
	// 		break;
	// 	case XK_Up:
	// 		std::cout << "up" << '\n';
	// 		pc->velocity = {0, 300.0};
	// 		pc->velocity = {0,0.0};
	// 		break;
	// 	case XK_s:
	// 		break;
	// 	case XK_Down:
	// 		break;
	// 	case XK_equal:
	// 		break;
	// 	case XK_minus:
	// 		break;
	// }
	return 0;
}

void physics()
{
	
}
// void render() {
//     // glClear(GL_COLOR_BUFFER_BIT);
// 	// Sprite* sc = s.getComponent<Sprite>(ptr);
// 	// Transform* tc = s.getComponent<Transform>(ptr);
// 	// std::shared_ptr<TextureInfo> ti = sc->textures["skip.png"];  // Reference to the unique_ptr
//     // if (ti == nullptr) {
// 	// 	exit(0);
// 	// }
//     // // Bind the texture directly from the unique_ptr
//     // glBindTexture(GL_TEXTURE_2D, *ti->texture);  // Dereference the unique_ptr to access GLuint

// 	// void show_jlo();
// 	// void show_balrowhany(Rect* r);

//     // int h = 64;
// 	// int w = 64;

// 	// int columns = 8;
// 	// int rows = 1;
// 	// int ix = frame % columns;
// 	// int iy = frame / columns;
// 	// frame++;
// 	// float fx = (float) ix / columns;
// 	// float fy = (float) iy / rows;

//     // // Draw the texture as a quad
//     // glBegin(GL_QUADS);
//     //     float xo = (float)1 / columns;
//     //     float xy = (float)1 / rows;
// 	// 	glTexCoord2f(fx + xo, fy + xy); glVertex2i(tc->pos[0] - w, tc->pos[1] - h);
// 	// 	glTexCoord2f(fx + xo, fy);      glVertex2i(tc->pos[0] - w, tc->pos[1] + h);
// 	// 	glTexCoord2f(fx, fy);           glVertex2i(tc->pos[0] + w, tc->pos[1] + h);
// 	// 	glTexCoord2f(fx, fy + xy);      glVertex2i(tc->pos[0] + w, tc->pos[1] - h);
//     // glEnd();

//     // glBindTexture(GL_TEXTURE_2D, 0);
//     // glDisable(GL_ALPHA_TEST);
// }




