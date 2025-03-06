#include "dchu.h"
#include <iostream>

//Credit Print Function
void show_dchu()
{
    std::cout << "Developer - David Chu\n";
}
void show_dchu(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Developer - David Chu");
}

//Inventory Class Functions
inventory::inventory()
{
    initStoreVolume(3, 4);
    full = false;
}

inventory::inventory(uint8_t x)
{
    initStoreVolume(x, x);
}

inventory::~inventory()
{
    for (uint8_t i = 0; i < sizeX; i++) {
        delete [] storage[i];
    }
    delete [] storage;
}

inventory::inventory(uint8_t x, uint8_t y)
{
    initStoreVolume(x, y);
}

void inventory::initStoreVolume(uint8_t x, uint8_t y)
{
    //Initialize Variables
    uint8_t sizeX = x;
    uint8_t sizeY = y;

    //Initialize Inventory Matrix
    storage = new item*[sizeX];
    for (uint8_t i = 0; i < sizeX; i++) {
        storage[i] = new item[sizeY];
    }

    //Pre-Load Item Slots
    if (sizeX > 0 && sizeY > 0) {
        uint8_t counter = 0;
        for (uint8_t i = 0; i < sizeX; i++) {
            for (uint8_t j = 0; j < sizeY; j++) {
                storage[i][j].item_name = ("Slot " + std::to_string(counter));
                storage[i][j].slot_num = ++counter;
            }
        }
    }
    //If DEVMODE, Run Unit Test
    #ifdef DEVMODE
        std::string test = "testItem";
        bool flag = false;
        for (uint8_t i = 0; i < sizeX; i++) {
            for (uint8_t j = 0; j < sizeY; j++) {
                storage[i][j].item_name = test;
            }
            
        }
        for (uint8_t i = 0; i < sizeX; i++) {
            for (uint8_t j = 0; j < sizeY; j++) {
                if(storage[i][j].item_name != test) {
                    flag = true;
                    break;
                }
            }
            if(flag) {
                break;
            }
        }
        if (!flag) {
            for (uint8_t i = 0; i < sizeX; i++) {
                for (uint8_t j = 0; j < sizeY; j++) {
                    std::cout << storage[i][j].item_name << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "Inventory Matrix - OK\n\n";
        } else {
            std::cout << "ERROR, Inventory Matrix not Loading";
        }
    #endif
}

//ECS: Inventory Management System
//void InventorySystem::update(Scene& scene, float dt)
//{}