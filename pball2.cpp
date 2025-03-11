//
//author:  Gordon Griesel
//date:    2018
//program: pball.cpp
//
//This program demonstrates a sphere constructed with polygons with texture.
//
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "fonts.h"
#include "image.h"
typedef float Flt;
typedef Flt Vec[3];
#define rnd() (float)rand() / (float)RAND_MAX
#define PI 3.14159265358979323846264338327950
Image img[1] = {

//"./13ball.gif",
"./resources/textures/planet2.gif"
};

class Global {
public:
	int xres, yres;
	GLuint pballTextureId;
	GLuint feltTextureId;
	GLuint standsTextureId;
	GLfloat lightAmbient[4];
	GLfloat lightDiffuse[4];
	GLfloat lightSpecular[4];
	GLfloat lightPosition[4];
	float pos[3];
	float vel[3];
	int lesson_num;
	float rtri;
	float rquad;
	float cubeRot[3];
	float cubeAng[3];
	float ballPos[3];
	float ballVel[3];
	float ballRot[3];
	float ballAng[3];
	float felt;
	Global() {
		srand(time(NULL));
		xres = 640;
		yres = 480;
		GLfloat la[]  = {  0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat ld[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat ls[] = {  0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat lp[] = { 100.0f, 60.0f, -140.0f, 1.0f };
		lp[0] = rnd() * 200.0 - 100.0;
		lp[1] = rnd() * 100.0 + 20.0;
		lp[2] = rnd() * 300.0 - 150.0;
		memcpy(lightAmbient, la, sizeof(GLfloat)*4);
		memcpy(lightDiffuse, ld, sizeof(GLfloat)*4);
		memcpy(lightSpecular, ls, sizeof(GLfloat)*4);
		memcpy(lightPosition, lp, sizeof(GLfloat)*4);
		float gpos[3]={20.0,200.0,0.0};
		float gvel[3]={3.0,0.0,0.0};
		memcpy(pos, gpos, sizeof(float)*3);
		memcpy(vel, gvel, sizeof(float)*3);
		lesson_num=0;
		rtri = 0.0f;
		rquad = 0.0f;
		float gcubeRot[3]={2.0,0.0,0.0};
		float gcubeAng[3]={0.0,0.0,0.0};
		memcpy(cubeRot, gcubeRot, sizeof(float)*3);
		memcpy(cubeAng, gcubeAng, sizeof(float)*3);
		float bp[3]={0.0,2.0,-7.0};
		float bv[3]={0.2,0.0,0.0};
		float ba[3]={0.0,0.0,0.0};
		memcpy(ballPos, bp, sizeof(float)*3);
		memcpy(ballVel, bv, sizeof(float)*3);
		memcpy(ballRot, ba, sizeof(float)*3);
		memcpy(ballAng, ba, sizeof(float)*3);
		felt = -1.1f;
	}
} g;

void init_textures(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics(void);
void render(void);

void init_textures(void)
{
	//create opengl texture elements
	glGenTextures(1, &g.pballTextureId);
	int w = img[0].width;
	int h = img[0].height;
	//
	glBindTexture(GL_TEXTURE_2D, g.pballTextureId);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
		GL_RGB, GL_UNSIGNED_BYTE, img[0].data);
	//
	//create opengl texture elements
	glGenTextures(1, &g.feltTextureId);
	w = img[1].width;
	h = img[1].height;
	//
	glBindTexture(GL_TEXTURE_2D, g.feltTextureId);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
		GL_RGB, GL_UNSIGNED_BYTE, img[1].data);
}

#define VecCross(a,b,c) \
(c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1]; \
(c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2]; \
(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]

void vecCrossProduct(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[1]*v1[2] - v1[1]*v0[2];
	dest[1] = v0[2]*v1[0] - v1[2]*v0[0];
	dest[2] = v0[0]*v1[1] - v1[0]*v0[1];
}

Flt vecDotProduct(Vec v0, Vec v1)
{
	return v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
}

void vecZero(Vec v)
{
	v[0] = v[1] = v[2] = 0.0;
}

void vecMake(Flt a, Flt b, Flt c, Vec v)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

void vecCopy(Vec source, Vec dest)
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
}

Flt vecLength(Vec v)
{
	return sqrt(vecDotProduct(v, v));
}

void vecNormalize(Vec v)
{
	Flt len = vecLength(v);
	if (len == 0.0) {
		vecMake(0,0,1,v);
		return;
	}
	len = 1.0 / len;
	v[0] *= len;
	v[1] *= len;
	v[2] *= len;
}

void vecSub(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[0] - v1[0];
	dest[1] = v0[1] - v1[1];
	dest[2] = v0[2] - v1[2];
}

//INIT OPENGL


// 			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
// 			glClearDepth(1.0);
// 			glDepthFunc(GL_LESS);
// 			glEnable(GL_DEPTH_TEST);
// 			glShadeModel(GL_SMOOTH);
// 			glMatrixMode(GL_PROJECTION);
// 			glLoadIdentity();
// 			gluPerspective(45.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f);
// 			glMatrixMode(GL_MODELVIEW);
// 			glLoadIdentity();
// gluLookAt(0,5,10,  0,0,0,  0,1,0);
// 			//Enable this so material colors are the same as vert colors.
// 			glEnable(GL_COLOR_MATERIAL);
// 			glEnable( GL_LIGHTING );
// 			glLightfv(GL_LIGHT0, GL_AMBIENT, g.lightAmbient);
// 			glLightfv(GL_LIGHT0, GL_DIFFUSE, g.lightDiffuse);
// 			glLightfv(GL_LIGHT0, GL_SPECULAR, g.lightSpecular);
// 			glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
// 			glEnable(GL_LIGHT0);
// 			break;
void DrawPoolball()
{
	static int firsttime=1;
	//16 longitude lines.
	//8 latitude levels.
	//3 values each: x,y,z.
	int i, j, i2, j2, j3;
	static Flt verts[9][16][3];
	static Flt norms[9][16][3];
	static Flt    tx[9][17][2];
	if (firsttime) {
		//build ball vertices here. only once!
		firsttime=0;
		Flt circle[16][2];
		Flt angle=0.0, inc = (PI * 2.0) / 16.0;
		// Start of optimizing
		//Attempting to Optimizing Sine and Cosine to reduce cost on computer
		for (i=0; i<16; i++) {
			circle[i][0] = cos(angle);
			circle[i][1] = sin(angle);
			angle -= inc;
			printf("circle[%2i]: %f %f\n", i, circle[i][0], circle[i][1]);
		}
		for (i=0; i<=8; i++) {
			for (j=0; j<16; j++) {
				verts[i][j][0] = circle[j][0] * circle[i][1]; 
				verts[i][j][2] = circle[j][1] * circle[i][1];
				verts[i][j][1] = circle[i][0];
				norms[i][j][0] = verts[i][j][0]; 
				norms[i][j][1] = verts[i][j][1];
				norms[i][j][2] = verts[i][j][2];
				tx[i][j][0] = (Flt)j / 16.0;
				tx[i][j][1] = (Flt)i / 8.0;
			}
			tx[i][j][0] = (Flt)j / 16.0;
			tx[i][j][1] = (Flt)i / 8.0;
		}
	}
	//
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	//position the camera
	gluLookAt(0,5,-4,  0,0,-7,  0,1,0);
	glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);

	glPushMatrix();
	glTranslatef(g.ballPos[0],g.ballPos[1],g.ballPos[2]);
	//glRotatef(cubeAng[0],1.0f,0.0f,0.0f);
	//glRotatef(cubeAng[1],0.0f,1.0f,0.0f);
	glRotatef(g.ballAng[2], 0.0f, 0.0f, 1.0f);
	//draw the ball, made out of quads...
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, g.pballTextureId);
	glBegin(GL_QUADS);
	for (i=0; i<8; i++) {
		for (j=0; j<16; j++) {
			i2 = i+1;
			j2 = (j+1) & 0x0f; //mod 16
			j3 = j+1;
			glNormal3fv(norms[i][j]);
			glTexCoord2fv(tx[i][j]); glVertex3fv(verts[i][j]);
			glNormal3fv(norms[i2][j]);
			glTexCoord2fv(tx[i2][j]); glVertex3fv(verts[i2][j]);
			glNormal3fv(norms[i2][j2]);
			glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
			glNormal3fv(norms[i][j2]);
			glTexCoord2fv(tx[i][j3]); glVertex3fv(verts[i][j2]);
		}
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//
	//draw the shadow
	glPushMatrix();
	glTranslatef(g.ballPos[0], 0.0, g.ballPos[2]);
	//
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(0,0,0,80);
	//
	glBegin(GL_QUADS);
	for (i=0; i<8; i++) {
		for (j=0; j<16; j++) {
			i2 = i+1;
			j2 = (j+1) & 15; //mod 16
			j3 = j+1;
			//vector from light to vertex
			Vec d[4];
			vecSub(g.lightPosition, verts[i ][j ], d[0]);
			vecSub(g.lightPosition, verts[i2][j ], d[1]);
			vecSub(g.lightPosition, verts[i2][j2], d[2]);
			vecSub(g.lightPosition, verts[i ][j2], d[3]);
			//don't let shadow pieces overlap
			//front or back facing?
			/*JC
			Vec v0,v1,v2;
			vecSub(verts[i2][j], verts[i][j], v0);
			vecSub(verts[i2][j], verts[i2][j2], v1);
			vecCrossProduct(v0,v1,v2);
			Flt dot = vecDotProduct(d[0], v2);
			if (dot >= 0.0)
				continue;
			//
			//where does each ray intersect the floor?
			Vec o = {g.lightPosition[0],g.lightPosition[1],g.lightPosition[2]};
			Vec p0 = {0.0, g.felt+.05f, 0.0};
			Vec norm = {0.0, 1.0, 0.0};
			//t = (p0 - o) . n / d . n
			Vec v, h;
			Flt dot1, dot2, t;
			for (int k=0; k<4; k++) {
				dot1 = vecDotProduct(d[k], norm);
				//if (dot1 == 0.0) break;
				vecSub(p0, o, v);
				dot2 = vecDotProduct(norm, v);
				//if (dot1 == 0.0) break;
				t = dot2 / dot1;
				//if (t < 0.0) break;
				//Hit point is along the ray
				//Calculate the hit point
				h[0] = o[0] + (d[k][0] * t);
				h[1] = o[1] + (d[k][1] * t);
				h[2] = o[2] + (d[k][2] * t);
				glVertex3fv(h);
				
			}*/
			//old code showing a shadow directly below ball
			//glVertex3f(verts[i ][j ][0], -1.0, verts[i ][j ][2]);
			//glVertex3f(verts[i2][j ][0], -1.0, verts[i2][j ][2]);
			//glVertex3f(verts[i2][j2][0], -1.0, verts[i2][j2][2]);
			//glVertex3f(verts[i ][j2][0], -1.0, verts[i ][j2][2]);
		}
	}
	glEnd();
	glDisable (GL_BLEND);
	glPopMatrix();
}

void physics(void)
{
	#define GRAVITY -0.4f
	if (g.lesson_num == 7) {
		//pool ball...

		//planet transform, physics
		g.ballVel[1] -= 0.005;
		float oldpos[2];
		oldpos[0] = g.ballPos[0];
		oldpos[1] = g.ballPos[1];
		g.ballPos[0] += g.ballVel[0];
		g.ballPos[1] += g.ballVel[1];
		//
		//felt resistance
		/* removed JC
		g.ballVel[0] *= 0.50;
		g.ballVel[1] *= 0.50;*/
		//new line of code 2
		//CONTROLS LOCATION?
		g.ballVel[0] *= 0.005;
		g.ballVel[1] *= 0.005;
		
		if (g.ballPos[1] != 0.0f) {
			g.ballPos[1] = 0.0f;
		}
		//
		//distance rolled?
		float d0,d1,dist;
		d0 = oldpos[0] - g.ballPos[0];
		d1 = oldpos[1] - g.ballPos[1];
		d0 = oldpos[0];
		d1 = oldpos[1];

		dist = sqrt(d0*d0 + d1*d1);


		//printf("op: %f %f np: %f %f dist: %f\n",
		//	oldpos[0], oldpos[1], g.ballPos[0], g.ballPos[1], dist);
		//fflush(stdout);
		//rotate ball speed
		float pct = dist * 2.0f;
		float sign = (g.ballVel[0] >= 0.0f) ? -1.0f : 1.0f;
		g.ballAng[2] += pct * sign;
	}
}

void render(void)
{
	//Rect r;
	glClear(GL_COLOR_BUFFER_BIT);
	//
	//Just draw a simple square

	glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f);
		glMatrixMode(GL_MODELVIEW);
		gluLookAt(0,5,10,  0,0,0,  0,1,0);
		//Enable this so material colors are the same as vert colors.
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
	DrawPoolball();
	//This sets 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	
}



