#ifndef JLO_H
#define JLO_H
#include <cmath>
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
        Vec2& operator+= (Vec2& v);
        Vec2& operator*= (float scale);
        float length() const;

};

class Entity 
{
    private:
        int id;
    
    public:
        Entity(int i);
        int getId() const;

};

struct Position
{
    Vec2 pos;
    Position(float x = 0.f, float y = 0.f);
};

struct Health
{
    int health;
    Health(int h = 0);
};


#endif JLO_H