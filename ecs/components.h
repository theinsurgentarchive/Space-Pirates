#pragma once

#include <string>
#include <cstdint>

enum Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    NORTH_EAST,
    SOUTH_EAST,
    NORTH_WEST,
    SOUTH_WEST
};

class Vec2 
{
    private:
        float pos[2];
        float length_squared() const;
    public:
        Vec2();
        Vec2(float x, float y);
        float getX();
        float getY();
        void setX(float x);
        void setY(float y);
        Vec2 operator-() const;
        float operator[] (int i) const;
        float& operator[] (int i);
        Vec2& operator+= (const Vec2& v);
        Vec2& operator*= (float scale);
        Vec2 operator* (float scale) const;
        float length() const;
};

class Transform
{
    public:
        Vec2 pos, scale;
        float rotation;
        Transform();
        Direction direction();
};

struct Sprite
{
    uint16_t width, height;
    std::string texture;
};

struct Physics
{
    Vec2 velocity;
    Vec2 acceleration;
};

class Health
{
    public:
        float hp, maxHp;
        Health();
        Health(float h, float m);
        float percent();
};

class AABBHitbox
{
    public:
        Vec2 corners[2];
        AABBHitbox(float centerX, float centerY, float radius);
        bool collided(AABBHitbox hitbox);
};
