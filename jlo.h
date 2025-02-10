#ifndef JLO_H
#define JLO_H
#include <cmath>
class Vector2 {
    private:
        float pos[2];
        float length_squared() const;
        
    public:
        Vector2() {}
        Vector2(float x, float y);
        float getX();
        float getY();
        void setX(float x);
        void setY(float y);
        Vector2 operator-() const;
        float operator[] (int i) const;
        float& operator[] (int i);
        Vector2& operator+= (Vector2& v);
        Vector2& operator*= (float scale);
        float length() const;

};

class IPositionable {
    protected:
        Vector2 pos;

    public:
        IPositionable() {}
        IPositionable(const Vector2& pos);
        const Vector2& getPosition() const;
        void setPosition(float x, float y);

};

class Player : public IPositionable {

};

#endif JLO_H