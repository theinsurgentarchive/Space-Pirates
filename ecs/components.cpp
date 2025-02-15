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