#include "math.h"
#include <cmath>

//Start - vec2f
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

Vec2 Vec2::operator+ (Vec2 v) const {
    return Vec2(pos[0] + v.pos[0], pos[1] + v.pos[1]);
}
Vec2 Vec2::operator-(Vec2 v) const {
    return Vec2(pos[0] - v.pos[0], pos[1] - v.pos[1]);
}
float Vec2::operator[] (int i) const {
    return pos[i];
}

float& Vec2::operator[] (int i) {
    return pos[i];
}

Vec2& Vec2::operator+= (const Vec2& v) {
    pos[0] += v.pos[0];
    pos[1] += v.pos[1];
    return *this;
}

Vec2& Vec2::operator*= (float scale) {
    pos[0] *= scale;
    pos[1] *= scale;
    return *this;
}

Vec2 Vec2::operator* (float scale) const {
    return Vec2(pos[0] * scale, pos[1] * scale);
}

float Vec2::length() const {
    return std::sqrt(length_squared());
}
float Vec2::length_squared() const {
    return pos[0] * pos[0] + pos[1] * pos[1];
}
//End - Vector

//start - vec2i
vec2i::vec2i() : pos{0,0} {}

vec2i::vec2i(int x, int y) : pos{x,y} {}

int vec2i::x () const {
    return pos[0];
}

int vec2i::y () const {
    return pos[1];
}

void vec2i::x(int x) {
    pos[0] = x;
}

void vec2i::y(int y) {
    pos[1] = y;
}

vec2i vec2i::operator-() const {
    return vec2i { -pos[0], -pos[1] };
}

int vec2i::operator[] (int i) const {
    return pos[i];
}

int& vec2i:: operator[] (int i) {
    return pos[i];
}

vec2i& vec2i::operator+= (const vec2i& v) {
    pos[0] += v.pos[0];
    pos[1] += v.pos[1];
    return *this;
}
//end - vec2i