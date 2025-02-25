#pragma once

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
        Vec2 operator+ (Vec2 v) const;
        Vec2 operator- (Vec2 v) const;
        float operator[] (int i) const;
        float& operator[] (int i);
        Vec2& operator+= (const Vec2& v);
        Vec2& operator*= (float scale);
        Vec2 operator* (float scale) const;
        float length() const;
};

class vec2i
{
    private:
        int pos[2];
    public:
        vec2i();
        vec2i(int x, int y);
        int x() const;
        int y() const;
        void x(int x);
        void y(int y);
        vec2i operator-() const;
        int operator[] (int i) const;
        int& operator[] (int i);
        vec2i& operator+= (const vec2i& v);
};