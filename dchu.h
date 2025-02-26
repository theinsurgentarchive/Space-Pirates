#pragma once
#include "jlo.h"
#include "jsandoval.h"
#include "balrowhany.h"
#include "mchitorog.h"
#include "fonts.h"

//Credit Print Function
void show_dchu(Rect*);

//Entity Components
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

struct item
{
    std::string item_name;
    uint16_t slot_num;
    /*TBD*/
};

//Declaration of Inventory Management
class inventory
{
    private:
        void initStoreVolume(uint8_t, uint8_t);
        bool full;
        item** storage;
        uint8_t sizeY;
        uint8_t sizeX;
        public:
        //Constructor
        inventory();
        inventory(uint8_t);
        inventory(uint8_t, uint8_t);

        //Destructor
        ~inventory();

        //Setters
        void addItem(item);
        void useItemSlot(/*TBD*/);

        //Getters:
        void getInventory();
        item returnItemSlot(uint8_t, uint8_t);
};

//ECS: Inventory Management System
class InventorySystem : public EntitySystem
{
    public:
        void update(Scene&, float);
};