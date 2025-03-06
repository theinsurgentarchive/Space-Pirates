#pragma once
#include "jlo.h"
#include "jsandoval.h"
#include "balrowhany.h"
#include "mchitorog.h"
#include "fonts.h"

//Credit Print Function
void ShowDChu(Rect*);

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
class Inventory
{
    private:
        void initStoreVolume(int, int);
        bool full;
        item** storage;
        vec2i inv_size;
        public:
        //Constructor
        Inventory();

        //Destructor
        ~Inventory();

        //Setters
        void addItem(item);
        void useItemSlot(/*TBD*/);

        //Getters:
        void getInventory();
        item returnItemSlot(int, int);
};

//ECS: Inventory Management System
namespace ecs
{
    class InventorySystem : public System
    {
        public:
            InventorySystem();
            void update(float dt) override;
    };
}
