#include "jlo.h"
#include <cmath>

//Start - Vec2
Vec2::Vec2() : pos{0,0} {}

Vec2::Vec2(float x, float y) : pos{x,y} {}

float Vec2::getX() {
    return pos[0];
}

float Vec2::getY() {
    return pos[1];
}

void Vec2::setX(float x) {
    pos[0] = x;
}

void Vec2::setY(float y) {
    pos[1] = y;
}

Vec2 Vec2::operator-() const {
    return Vec2(-pos[0],-pos[1]);
}

float Vec2::operator[] (int i) const {
    return pos[i];
}

float& Vec2::operator[] (int i) {
    return pos[i];
}

Vec2& Vec2::operator+= (Vec2& v) {
    pos[0] += v.pos[0];
    pos[1] += v.pos[1];
    return *this;
}

Vec2& Vec2::operator*= (float scale) {
    pos[0] *= scale;
    pos[1] *= scale;
    return *this;
}

float Vec2::length() const {
    return std::sqrt(length_squared());
}
float Vec2::length_squared() const {
    return pos[0] * pos[0] + pos[1] * pos[1];
}
//End - Vec2 

//Start - Entity
Entity::Entity(int i) : id(i) {}

int Entity::getId() const {
    return id;
}
//End - Entity

//Start - Health
Health::Health(int h) : health(h) {}
//End - Health

//Start - Position
Position::Position(float x, float y) : pos(x,y) {}
//End - Position


