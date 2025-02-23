#pragma once

#include "math.h"
#include "anim.h"
#include "textures.h"
#include <memory>
#include <vector>
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

class Transform
{
    public:
        Vec2 pos, scale;
        float rotation;
        Transform();
        Direction direction();
};

class Sprite
{
    public:
        std::unordered_map<std::string,std::shared_ptr<Animation>> animations;
        uint16_t render_order;
        std::string c_anim;
};

struct Physics
{
    Vec2 velocity;
    Vec2 acceleration;
    bool physics {true};
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

//@theinsurgentarchive
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
        void setOxygenMod(float);
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