#include "components.h"

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
