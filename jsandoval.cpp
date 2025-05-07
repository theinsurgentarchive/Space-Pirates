#include "jsandoval.h"
#include "jlo.h"
#include <iostream>
#include <GL/gl.h>
#include <random>
#include <vector>
#include <cmath>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

namespace ecs 
{
    extern ECS ecs;
Entity* character_x()
{
    //ecs::ecs.component().bulkAssign<PHYSICS,SPRITE,TRANSFORM,HEALTH>(ptr); 
    auto x = ecs::ecs.entity().checkout();
    auto [health,transform] = ecs::ecs.component().assign<HEALTH,TRANSFORM>(x);
    if (health) {
        health -> health = 50;
        health -> max = 100;
    }
    //bring in xres yres to file
    if (transform) {
        transform -> pos[0] = 20;
        transform -> pos[1] = 30;
    }

    auto [retrievedHealth, retrievedPos] = ecs::ecs.component().fetch<HEALTH,TRANSFORM>(x);
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
Entity* GeneratePlanet(int xres, int yres)
{
    float rndNums[4];
    float rndCoor[3];
    PlanetSeedGenerator(rndNums);
    PlanetCoorGenerator(rndCoor, xres, yres);

    auto planetEntity = ecs::ecs.entity().checkout();
    ecs::ecs.component().assign<PLANET>(planetEntity);

    auto [properties] = ecs::ecs.component().fetch<PLANET>(planetEntity);
    
    //gave up on being organized
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float>dis(-1.0f, 1.0f);


    properties->size = PlanetSize(rndNums[0]);
    properties->smooth = PlanetSmooth(rndNums[1]);
    properties->temperature = PlanetTemp(rndNums[2]);
    properties->humidity = PlanetHumidity(rndNums[3]);

    properties-> AngY  = dis(gen);
    properties-> PosX = rndCoor[0];
    properties-> PosY = rndCoor[1];
    properties-> PosZ = rndCoor[2];
    properties-> rotationX = dis(gen);
    properties-> rotationY = dis(gen);
    //GLfloat lightPosition[] = { 100.0f, 60.0f, -140.0f, 1.0f};

    if (properties) {
        std::cout << "Planet Properties: " << "Size: "<< properties -> size
        << " Smooth: " << properties -> smooth << " Temperature: " << 
        properties-> temperature << " humidity: " << properties-> humidity << 
        " AngY: " << properties-> AngY << " PosX: " << properties-> PosX << 
        " PosY: " << properties-> PosY << " PosZ: " << properties-> PosZ << 
        " rotationX: " << properties-> rotationX << " rotationY: " << 
        properties-> rotationY << std::endl;
    }
    else {
        std::cout << "Failed to retrieve Health Component." <<std::endl;
    }

    return planetEntity;

}
void createPlanets(int xres, int yres)
{
    for (int i = 0; i < 6; i++) {
        GeneratePlanet(xres, yres);
    }
}
void DrawPlanets(GLfloat* lightPosition)
{
    auto planets = ecs::ecs.query<PLANET>();
    for (auto* entity:planets){
        auto [planet] = ecs.component().fetch<PLANET>(entity);
        DrawPlanet(planet-> AngY, planet-> PosX, planet-> PosY, planet-> 
            PosZ, lightPosition, planet->size, planet->rotationX, 
            planet->rotationY, planet->smooth, planet->temperature);
    }
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
void DrawPlanetMenu(float planetAngY, float planetPosX, float planetPosY, 
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

//////////////////////////// PLANET COMPONENTS BELOW ////////////////////////
void PlanetSeedGenerator(float values[4]) 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 999);

    //Planet Properties
    values[0] = dis(gen); //Size
    values[1] = dis(gen); //Smooth
    values[2] = dis(gen); //Temp
    values[3] = dis(gen); //Humidity
}

void PlanetCoorGenerator(float values[3], int xres, int yres)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sideDist(0,3);
    std::uniform_int_distribution<> offsetDis(0,30);

    int width = xres/100.0f;
    int height = yres/100.0;
    int side = sideDist(gen);
    float x;
    float y;

    std::uniform_int_distribution<>yDist(0, height);
    std::uniform_int_distribution<>xDist(0, width);

    switch (side) {
        case 0: {
            x = -xDist(gen) - offsetDis(gen);
            y = yDist(gen) + offsetDis(gen);
            break; 
        }
        case 1: {
            x = xDist(gen)+ offsetDis(gen);
            y = yDist(gen) + offsetDis(gen);
            break;
        }
        case 2: {
            x = xDist(gen)+ offsetDis(gen);
            y = -yDist(gen) - offsetDis(gen);
            break;
        }
        case 3: {
            x = -xDist(gen) - offsetDis(gen);
            y = -yDist(gen) - offsetDis(gen);
            break;   
        }        
    }    
  
    float z = -10; //MAYBE change it later

    values[0] = x; 
    values[1] = y;
    values[2] = z;

}

float PlanetSize(float rndNum)
{
    //chance rndNum to a % probability to define Size (0.5x to 3.5x)
    float sizeChance = ((rndNum - 100) / 899) * 100;
    
    if (sizeChance > 92.5)
        return 3.5f;
    else if (sizeChance > 82.5)
        return 2.5f;
    else if (sizeChance > 67.5)
        return 2.0f;
    else if (sizeChance > 45.0)
        return 1.50f;
    else if (sizeChance > 12.5)
        return 1.0f;
    else
        return 0.75f;
}

float PlanetSmooth([[maybe_unused]]float rndNum)
{
    return ((rndNum - 100) / 899) * 100;
}

float PlanetTemp(float rndNum)
{
    //Scaling rndNum to Temp
    float temp = -30 + ((float(rndNum) - 100) / 899.0) * (100 + 30);
    return temp;
}

float PlanetHumidity(float rndNum)
{
    float humidity = (float(rndNum) - 100) / 899.0;
    return humidity;
}

/*Imitating Perlin-Noise - Had to do research to understand concept and
best way to imitate without Library. One guide: Perlin C++ SFML
Tutorial - Youtube
*/
Vec2<float> GetGradient(int x, int y) 
{
    int hash = (x * 1836311903) ^ (y * 2971215073);
    hash = (hash >> 13) ^ hash;
    float angle = (hash % 360) * 3.14159 / 180.0;
    return Vec2<float>(std::cos(angle), std::sin(angle));
}
//found on reddit: which is introduced by Ken Perlin
float SmoothInterpo(float t) 
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

//Grab coordinates and determine dot based on vector distance
float PerlinNoise(float x, float y) 
{
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float sx = x - (float)x0;
    float sy = y - (float)y0;

    Vec2<float> c00 = GetGradient(x0, y0);
    Vec2<float> c10 = GetGradient(x1, y0);
    Vec2<float> c01 = GetGradient(x0, y1);
    Vec2<float> c11 = GetGradient(x1, y1);

    Vec2<float> dist00(sx, sy);
    Vec2<float> dist10(sx - 1, sy);
    Vec2<float> dist01(sx, sy - 1);
    Vec2<float> dist11(sx - 1, sy - 1);

    float dot00 = c00.dot(dist00);
    float dot10 = c10.dot(dist10);
    float dot01 = c01.dot(dist01);
    float dot11 = c11.dot(dist11);

    float u = SmoothInterpo(sx);
    float v = SmoothInterpo(sy);
    float i1 = (1 - u) * dot00 + u * dot10;
    float i2 = (1 - u) * dot01 + u * dot11;
    return (1 - v) * i1 + v * i2;
}

//Generate Height Map (Creating the grid for the Height Map 9 x 16)
std::vector<float> GenerateHeightMap() 
{
    std::vector<float> heightMap(144);

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 9; j++) {
            float x = ((float)j / 9.0f) * 5.0f;
            float y = ((float)i / 16.0f) * 5.0f;

            float pNoise = PerlinNoise(x, y);
            pNoise = (pNoise + 1.0f) / 2.0f;
            heightMap[i * 9 + j] = pNoise;
        }
    }
    return heightMap;
}

//Return color based on Temp
glm::vec3 TempToColor(float temp) {
    if (temp < 20.0f) {
        return glm::vec3(0.5f, 0.7f, 1.0f); 
    }
    else if (temp < 30) {
        float factor = (temp - 20.0f) / 10.0f;
        glm::vec3 icy = glm::vec3(0.0f, 0.0f, 0.5f);
        glm::vec3 cool = glm::vec3(0.0f, 0.3f, 0.1f);
        return glm::mix(icy, cool, factor);
    }
    else if (temp < 60.0f) {
        float factor = (temp - 30.0f) / 30.0f;
        glm::vec3 water = glm::vec3(0.0f, 0.0f, 0.3f);
        glm::vec3 warm = glm::vec3(0.3f, 0.6f, 0.2f);
        return glm::mix(water, warm, factor);
    }
    else {
        float factor = (temp - 60.0f) / 40.0f;
        glm::vec3 warm = glm::vec3(0.3f, 0.6f, 0.2f);
        glm::vec3 hot = glm::vec3(1.0f, 0.0f, 0.0f);
        return glm::mix(warm, hot, glm::clamp(factor, 0.0f, 1.0f));
    }
}

//FINALLY ADJUST MenuPlanet to General Planet
void DrawPlanet(float planetAngY, float planetPosX, float planetPosY, 
    float planetPosZ, GLfloat* lightPosition, float size, 
    float rotationX, float rotationY, [[maybe_unused]] float roughness, 
    float temp) 
{
    std::vector<float> heightMap = GenerateHeightMap(); 
    static int firsttime = 1;
    [[maybe_unused]]int i, j, i2, j2, j3;
    static Flt verts[9][16][3];
    static Flt norms[9][16][3];
    [[maybe_unused]]static Flt tx[9][17][2];

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
                int index = i * 16 + j;
                // Apply roughness TEST LATER
                //float height = heightMap[index] * ((roughness)/100);  
                float height = heightMap[index];
                //testing noise
                float noise = PerlinNoise(i, j);
                verts[i][j][0] = (circle[j][0] * circle[i][1]) * 
                (1.0f + height + noise);
                verts[i][j][2] = (circle[j][1] * circle[i][1]) * 
                (1.0f + height + noise);
                verts[i][j][1] = circle[i][0] * (1.0f + height + noise);

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
    glm::vec3 color = TempToColor(temp); 
    glColor3f(color.r, color.g, color.b);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glPushMatrix();
    glTranslatef(planetPosX, planetPosY-5, planetPosZ);
    glScalef(size, size, size);
    glRotatef(planetAngY, rotationX, rotationY, 0.0f);
    glBegin(GL_QUADS);

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 16; j++) {
            i2 = i + 1;
            j2 = (j + 1) & 0x0f;
            j3 = j + 1;
            glNormal3fv(norms[i][j]);
            glVertex3fv(verts[i][j]);
            glNormal3fv(norms[i2][j]);
            glVertex3fv(verts[i2][j]);
            glNormal3fv(norms[i2][j2]);
            glVertex3fv(verts[i2][j2]);
            glNormal3fv(norms[i][j2]);
            glVertex3fv(verts[i][j2]);
        }
    }

    glEnd();
    glPopMatrix();

}


//Planet Collision
bool PlanetCollision(const ecs::Entity* Planet) {

    auto [traits] = ecs::ecs.component().fetch<ecs::Planet>(Planet);

    if (!traits){
        std::cout << "We are missing components for collision" << std::endl;
    }

    float dx = -traits->PosX;
    float dy = -traits->PosY;

    return (((dy <= 1) && (dy >= -1)) && ((dx >= - 1) && (dx <= 1)));
    //gl.state = PLAYING;
}

