#include "jsandoval.h"
#include "jlo.h"
#include <iostream>
#include <GL/gl.h>


#define rnd() (float)rand() / (float)RAND_MAX
typedef float Flt;
typedef float Vec[3];
#define PI 3.14159265358979323846264338327950

void vecSub(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[0] - v1[0];
	dest[1] = v0[1] - v1[1];
	dest[2] = v0[2] - v1[2];
}

namespace ecs {
    extern ECS ecs;
Entity* character_x()
{
    
    auto x = ecs::ecs.entity().checkout();
    auto health = ecs::ecs.component().assign<HEALTH>(x);
    auto transform = ecs::ecs.component().assign<TRANSFORM>(x);
    
    if (health) {
        health -> health = 50;
        health -> max = 100;
    }
    //bring in xres yres to file
    if (transform) {
        transform -> pos[0] = 20;
        transform -> pos[1] = 30;
    }

    auto retrievedHealth = ecs::ecs.component().fetch<HEALTH>(x);
    auto retrievedPos = ecs::ecs.component().fetch<TRANSFORM>(x);
    if (retrievedPos) {
        std::cout << "Position on x-axis: " << retrievedPos -> pos[0] << " y Position " << retrievedPos -> pos[1] << std::endl;
    }
    else {
        std::cout << "Failed to retrieve Health Component." <<std::endl;
    }
    if (retrievedHealth) {
        std::cout << "Health: " << retrievedHealth -> health << std::endl;
        std::cout << "Max Health: " << retrievedHealth -> max << std::endl;
    }
    else {
        std::cout << "Failed to retrieve Health Component." <<std::endl;
    }
    return x;
}
}
void DrawPlanet(float planetAngY, float planetPosX, float planetPosY, float planetPosZ, GLfloat* lightPosition, GLuint planetTexture) {

    static int firsttime = 1;
    // 16 longitude lines, 9 latitude levels, 3 values each: x, y, z.
    int i, j, i2, j2, j3;
    static Flt verts[9][16][3];
    static Flt norms[9][16][3];
    static Flt tx[9][17][2];

    if (firsttime) {
        firsttime = 0;

        static const Flt circle[16][2] = {
            {1.000000f, 0.000000f}, {0.923880f, -0.382683f}, {0.707107f, -0.707107f},
            {0.382683f, -0.923880f}, {-0.000000f, -1.000000f}, {-0.382683f, -0.923880f},
            {-0.707107f, -0.707107f}, {-0.923880f, -0.382683f}, {-1.000000f, -0.000000f},
            {-0.923880f, 0.382683f}, {-0.707107f, 0.707107f}, {-0.382683f, 0.923880f},
            {0.000000f, 1.000000f}, {0.382683f, 0.923880f}, {0.707107f, 0.707107f},
            {0.923880f, 0.382683f}
        };
        for (i = 0; i <= 8; i++) {
            for (j = 0; j < 16; j++) {
                verts[i][j][0] = circle[j][0] * circle[i][1];
                verts[i][j][2] = circle[j][1] * circle[i][1];
                verts[i][j][1] = circle[i][0];

                norms[i][j][0] = verts[i][j][0];
                norms[i][j][1] = verts[i][j][1];
                norms[i][j][2] = verts[i][j][2];

                tx[i][j][0] = (Flt)j / 16.0f;
                tx[i][j][1] = (Flt)i / 8.0f;
            }
            tx[i][j][0] = (Flt)j / 16.0f;
            tx[i][j][1] = (Flt)i / 8.0f;
        }
    }
    
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    // Set up the camera
    gluLookAt(0,5,-4,  0,0,-7,  0,1,0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glPushMatrix();
    //glTranslatef(0.0,0.0,-5.0f);
    glTranslatef(planetPosX, planetPosY, planetPosZ);
    // Scale the ball (this controls its size)
    glScalef(0.5f, 0.5f, 0.5f);
    // Rotate the ball around the z-axis
    glRotatef(planetAngY, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0, 1.0, 1.0);
    //glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, planetTexture);
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

    glPushMatrix();

    glTranslatef(planetPosX, 0.0, planetPosZ);

    //glDisable(GL_TEXTURE_2D);
    //glPushAttrib(GL_ENABLE_BIT);
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
            vecSub(lightPosition, verts[i ][j ], d[0]);
            vecSub(lightPosition, verts[i2][j ], d[1]);
            vecSub(lightPosition, verts[i2][j2], d[2]);
            vecSub(lightPosition, verts[i ][j2], d[3]);
        }
    }
    glEnd();
    //glPopAttrib();
    glDisable (GL_BLEND);
    glPopMatrix();
    
    }

