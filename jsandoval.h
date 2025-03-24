#pragma once
#include "balrowhany.h"
#include "mchitorog.h"
#include "fonts.h"
#include "jlo.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include "image.h"    
#include <math.h>
#include <random>

namespace ecs 
{
    Entity* character_x();
}

class Planet 
{
    GLfloat lightPosition[4];
    float planetPos[3];
    float planetRot[3];
    float planetAng[3];
    GLuint planetTexture;

    Planet();
    void setLightPosition(GLfloat x, GLfloat y, GLfloat z);
    void setPlanetPosition(float x, float y, float z);
    void setPlanetRotation(float x, float y, float z);
    void setPlanetAngle(float angle);
};

void DrawPlanet(float planetAngY, float planetPosX, float planetPosY, 
    float planetPosZ, GLfloat* lightPosition, GLuint planetTexture, float size, 
    float rotationX, float rotationY);

void DisableFor2D();
void EnableFor3D();

//Used for Planet Generation and Map Generation
int* PlanetSeedGenerator();
float PlanetSize(int rndNum);
float PlanetSmooth(int rndNum);
float PlanetTemp(int rndNum);
float GeneratePlanet(int* rndNums);