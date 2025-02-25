#include "components.h"
#ifdef DEVMODE
    #include <iostream>
#endif

//Start - Transform
Transform::Transform() : pos{0,0},scale{0,0},rotation{0}{}

Direction Transform::direction() {
    return NORTH;
}
//End - Transform
//Start - Health
Health::Health() : hp{0},maxHp{0} {}

Health::Health(float h, float m) : hp{h},maxHp{m} {}

float Health::percent() {
    return hp / maxHp;
}
//End - Health

//Start - AABBHitbox
AABBHitbox::AABBHitbox(float centerX, float centerY, float radius) {
    corners[0] = Vec2(centerX - radius, centerY - radius);
    corners[1] = Vec2(centerX + radius, centerX + radius);
}
//TODO: implement collided 

//End - AABBHitbox

//@theinsurgentarchive
uint8_t upgrade::getLevel()
{
    return level;
}
uint8_t upgrade::getHealthMod()
{
    return health_modifier;
}
float* upgrade::getUpgradableMods()
{
    float temp[5];
    temp[0] = health_modifier;
    temp[1] = speed_modifier;
    temp[2] = damage_modifier;
    temp[3] = fuel_modifier;
    temp[4] = oxygen_modifier;
    return temp;
}

std::string upgrade::getDifficulty()
{
    return difficulty_modifier;
}

void upgrade::setDamageMod(float mod)
{
    damage_modifier = mod;
}

void upgrade::setOxygenMod(float mod)
{
    oxygen_modifier = mod;
}

void upgrade::setFuelMod(float mod)
{
    fuel_modifier = mod;
}

void upgrade::setSpeedMod(float mod)
{
    speed_modifier = mod;
}

inventory::inventory()
{
    initStoreVolume(3, 4);
    full = false;
}

inventory::inventory(uint8_t x)
{
    initStoreVolume(x, x);
}

inventory::~inventory()
{
    for (uint8_t i = 0; i < sizeX; i++) {
        delete [] storage[i];
    }
    delete [] storage;
}

inventory::inventory(uint8_t x, uint8_t y)
{
    initStoreVolume(x, y);
}

void inventory::initStoreVolume(uint8_t x, uint8_t y)
{
    //Initialize Variables
    uint8_t sizeX = x;
    uint8_t sizeY = y;

    //Intialize Inventory Matrix
    storage = new item*[sizeX];
    for (uint8_t i = 0; i < sizeX; i++) {
        storage[i] = new item[sizeY];
    }

    //Pre-Load Item Slots
    if (sizeX > 0 && sizeY > 0) {
        uint8_t counter = 0;
        for (uint8_t i = 0; i < sizeX; i++) {
            for (uint8_t j = 0; j < sizeY; j++) {
                storage[i][j].item_name = ("Slot " + std::to_string(counter));
                storage[i][j].slot_num = ++counter;
            }
        }
    }
    //If DEVMODE, Run Unit Test
    #ifdef DEVMODE
        std::string test = "testItem";
        bool flag = false;
        for (uint8_t i = 0; i < sizeX; i++) {
            for (uint8_t j = 0; j < sizeY; j++) {
                storage[i][j].item_name = test;
            }
            
        }
        for (uint8_t i = 0; i < sizeX; i++) {
            for (uint8_t j = 0; j < sizeY; j++) {
                if(storage[i][j].item_name != test) {
                    flag = true;
                    break;
                }
            }
            if(flag) {
                break;
            }
        }
        if (!flag) {
            for (uint8_t i = 0; i < sizeX; i++) {
                for (uint8_t j = 0; j < sizeY; j++) {
                    std::cout << storage[i][j].item_name << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "Inventory Matrix - OK\n\n";
        } else {
            std::cout << "ERROR, Inventory Matrix not Loading";
        }
    #endif
}
