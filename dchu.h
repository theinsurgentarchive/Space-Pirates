#pragma once
#include <string>
#include "asteroids/fonts.h"
#include "jlo.h"

//Credit Print Function
void show_dchu(Rect*);

class upgrade
{
    private:
        //Current Level
        float level;
        uint8_t current_level;
	    uint8_t max_level;
        //modifiers
	    float difficulty_modifier;
        float health_modifier;
        float speed_modifier;
        float damage_modifier;
        float fuel_modifier;
        float oxygen_modifier;
    public:
        //Constructors
        upgrade();
        
        //Getters
        uint8_t getLevel();
        uint8_t getHealthMod();
        float getUpgradableMods();

        //Setters
        void setDamageMod(float);
        void setOxygenMod();
        void setFuelMod(float);
        void setSpeedMod(float);
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