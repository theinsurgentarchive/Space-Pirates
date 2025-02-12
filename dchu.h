#ifndef DCHU_H
#define DCHU_H
#include <string>
#include "jlo.h"

class planetBase : public Entity
{
    protected:
        //Inheritables
        std::string type;
        Sprite img;
        color rgb;
        Transform psr;
        bool is_exhausted;
        virtual void randomizePlanet() = 0;
        void setSprite(Sprite);
        void setTransform(Transform);
    public:
        /*Constructors*/
        //Intialize Random Planet
        planetBase(EntityID, ComponentMask);
        //Intialize Pre-defined Planet
        planetBase(EntityID, ComponentMask, std::string, Sprite, color, 
                                                                    Transform);

        /*Getters*/
        std::string getType();
        Vec2 getPosition();
        
        /*Setters*/
        void exhausted();
};
class color
{
    private:
        uint8_t red;
        uint8_t blue;
        uint8_t green;
    public:
        /*Constructors*/
        color();
        color(uint8_t, uint8_t, uint8_t);
        
        /*Getters*/
        uint8_t getRed();
        uint8_t getBlue();
        uint8_t getGreen();

        /*Setters*/
        void setColors(uint8_t, uint8_t, uint8_t);
        void setColors(color);
        void setRed(uint8_t);
        void setBlue(uint8_t);
        void setGreen(uint8_t);
};
#endif