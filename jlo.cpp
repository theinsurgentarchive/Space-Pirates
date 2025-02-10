#include "jlo.h"
#include <cmath>

//Start - Vector2
Vector2::Vector2() : pos{0,0} {}

Vector2::Vector2(float x, float y) : pos{x,y} {}

float Vector2::getX() {
    return pos[0];
}

float Vector2::getY() {
    return pos[1];
}

void Vector2::setX(float x) {
    pos[0] = x;
}

void Vector2::setY(float y) {
    pos[1] = y;
}

Vector2 Vector2::operator-() const {
    return Vector2(-pos[0],-pos[1]);
}

float Vector2::operator[] (int i) const {
    return pos[i];
}

float& Vector2::operator[] (int i) {
    return pos[i];
}

Vector2& Vector2::operator+= (Vector2& v) {
    pos[0] += v.pos[0];
    pos[1] += v.pos[1];
    return *this;
}

Vector2& Vector2::operator*= (float scale) {
    pos[0] *= scale;
    pos[1] *= scale;
    return *this;
}

float Vector2::length() const {
    return std::sqrt(length_squared());
}
float Vector2::length_squared() const {
    return pos[0] * pos[0] + pos[1] * pos[1];
}
//End - Vector2 

//Start - Positionable 
IPositionable::IPositionable() : pos(0,0) {}

IPositionable::IPositionable(const Vector2& pos) : pos(pos) {}

const Vector2& IPositionable::getPosition() const {
    return pos;
}

void IPositionable::setPosition(float x, float y) {
    pos.setX(x);
    pos.setY(y);
}
//End - Positionable





