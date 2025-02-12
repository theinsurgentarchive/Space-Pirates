#ifndef DCHU_H
#define DCHU_H
#include <string>
#include "jlo.h"

/*
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
        //Constructors
            //Intialize Random Planet
        planetBase(EntityID, ComponentMask);
            //Intialize Pre-defined Planet
        planetBase(EntityID, ComponentMask, std::string, Sprite, color, 
                                                                    Transform);

        //Getters
        std::string getType();
        Vec2 getPosition();
        
        //Setters
        void exhausted();
};
*/

struct ent_type
{
    std::string type;
};

struct upgrade
{
    //Current Level
    uint8_t level;
	uint8_t max_level;
    //modifiers
	uint8_t level_modifier;
    uint8_t health_modifier;
    float speed_modifier;
    float damage_modifier;
    float fuel_modifier;
    float oxygen_modifier;
};

struct oxygen_resource
{
	//Oxygen Level
	float oxygen;
	//Max O2 Level
	float max;
    //Is O2 Depleted?
	bool depleted;
};

struct fuel_resource
{
	//Fuel Level
	float fuel;
	//Max Fuel Level
	float max;
    //Is Fuel Depleted?
	bool depleted;
};

struct color
{
    uint8_t red;
    uint8_t blue;
    uint8_t green;
    uint8_t alpha;
};
#endif