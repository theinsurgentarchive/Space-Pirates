#include "jsandoval.h"
#include "jlo.h"
#include <iostream>
#include <GL/gl.h>
#include <random>
#include <vector>

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
        std::cout << "Position on x-axis: " << retrievedPos -> pos[0] 
        << " y Position " << retrievedPos -> pos[1] << std::endl;
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
void DisableFor2D()
{
    glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
}
void EnableFor3D()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}
void DrawPlanet(float planetAngY, float planetPosX, float planetPosY, 
    float planetPosZ, GLfloat* lightPosition, GLuint planetTexture, float size, 
    float rotationX, float rotationY) 
{
    
    static int firsttime = 1;
    // 16 longitude lines, 9 latitude levels, 3 values each: x, y, z.
    int i, j, i2, j2, j3;
    static Flt verts[9][16][3];
    static Flt norms[9][16][3];
    static Flt tx[9][17][2];

    if (firsttime) {
        firsttime = 0;

        static const Flt circle[16][2] = {
            {1.000000f, 0.000000f}, {0.923880f, -0.382683f}, 
            {0.707107f, -0.707107f}, {0.382683f, -0.923880f}, 
            {-0.000000f, -1.000000f}, {-0.382683f, -0.923880f},
            {-0.707107f, -0.707107f}, {-0.923880f, -0.382683f}, 
            {-1.000000f, -0.000000f}, {-0.923880f, 0.382683f}, 
            {-0.707107f, 0.707107f}, {-0.382683f, 0.923880f},
            {0.000000f, 1.000000f}, {0.382683f, 0.923880f}, 
            {0.707107f, 0.707107f}, {0.923880f, 0.382683f}
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
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glPushMatrix();
    //glTranslatef(0.0,0.0,-5.0f);
    glTranslatef(planetPosX, planetPosY-5, planetPosZ);
    // Scale the ball (this controls its size)
    glScalef(size, size, size);
    // Rotate the ball around the z-axis
    glRotatef(planetAngY, rotationX, rotationY, 0.0f);
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

    //This adds shadow
    glPushMatrix();
    glTranslatef(planetPosX, 0.0, planetPosZ);

    //glDisable(GL_TEXTURE_2D);
    glPushAttrib(GL_ENABLE_BIT);
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
    glPopAttrib();
    //glDisable (GL_BLEND);
    glPopMatrix();
    
}

int* PlanetSeedGenerator() 
{
    static int values[3];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 999);

    //First 3 numbers represent size, 2nd smoooth, 3rd temp
    values[0] = dis(gen);
    values[1] = dis(gen);
    values[2] = dis(gen);

    return values;
}

float PlanetSize(int rndNum)
{
    if (rndNum > 934)
        return 5.0f;
    else if (rndNum > 844)
        return 2.0f;
    else if (rndNum > 709)
        return 1.5f;
    else if (rndNum > 506)
        return 1.0f;
    else if (rndNum > 213)
        return 0.75f;
    else
        return 0.5f;
}

float PlanetSmooth([[maybe_unused]]int rndNum)
{
    return 0.0f;
}

float PlanetTemp(int rndNum)
{
    if (rndNum > 934) {}
    return 0.0f; 
}

float GeneratePlanet(int* rndNums)
{
    [[maybe_unused]]float size = PlanetSize(rndNums[0]);
    [[maybe_unused]]float smooth = PlanetSmooth(rndNums[1]);
    [[maybe_unused]]float temp = PlanetTemp(rndNums[2]);

//Implement Entity Calling and set up Biome Attribute thru here
//define _MAX_TEMPERATURE 100.0f
//define _MIN_TEMPERATURE -30.0f
    return 0.0f;
}