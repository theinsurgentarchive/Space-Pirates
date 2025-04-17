#pragma once
#include "balrowhany.h"
#include "mchitorog.h"
#include "fonts.h"
#include "jlo.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/vec3.hpp>
#include <stdio.h>
#include "image.h"    
#include <random>

namespace ecs
{
    Entity* character_x();
    Entity* GeneratePlanet();
    struct PLANET
    {
        float temperature;
        float humidity;
        float roughness;
        float size;
        float smooth;
        float rotationX;
        float rotationY;
        float AngY;
        float PosX;
        float PosY;
        float PosZ;
    };
    void updatePlanetSpin();
}



void DrawPlanetMenu(float planetAngY, float planetPosX, float planetPosY,
    float planetPosZ, GLfloat* lightPosition, GLuint planetTexture, float size,
    float rotationX, float rotationY);

void DisableFor2D();
void EnableFor3D();

//Used for Planet Generation and HeightMap Generation
void PlanetSeedGenerator(float values[4]);
void PlanetCoorGenerator(float values[3]);
float PlanetSize(float rndNum);
float PlanetSmooth(float rndNum);
float PlanetTemp(float rndNum);
float PlanetHumidity(float rndNum);
// void GeneratePlanet();

//Height Map Generation
Vec2<float> GetGradient(int x, int y);
float SmoothInterpo(float t);
float PerlinNoise(float x, float y);
std::vector<float> GenerateHeightMap();
glm::vec3 TempToColor(float temp);
void DrawPlanet(float planetAngY, float planetPosX, float planetPosY,
    float planetPosZ, GLfloat* lightPosition, float size,
    float rotationX, float rotationY, float roughness, float temp);
