#include "dchu.h"
#include <iostream>

//Credit Print Functions
//Console print
void ShowDChu()
{
    std::cout << "Developer - David Chu\n";
}

//Window render
void ShowDChu(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Developer - David Chu");
}

//Inventory Class Functions
Inventory::Inventory()
{
    initStoreVolume(3, 4);
    full = false;
}

Inventory::~Inventory()
{
    for (uint8_t i = 0; i < size.x(); i++) {
        delete [] storage[i];
    }
    delete [] storage;
}

void Inventory::initStoreVolume(uint8_t x, uint8_t y)
{
    //Initialize Variables
    size.x(x);
    size.y(y);

    //Initialize Inventory Matrix
    storage = new item*[size.x()];
    for (uint8_t i = 0; i < size.x(); i++) {
        storage[i] = new item[size.y()];
    }

    //Pre-Load Item Slots
    if (size.x() > 0 && size.y() > 0) {
        uint8_t counter = 0;
        for (uint8_t i = 0; i < size.x(); i++) {
            for (uint8_t j = 0; j < size.y(); j++) {
                storage[i][j].item_name = ("Slot " + std::to_string(counter));
                storage[i][j].slot_num = ++counter;
            }
        }
    }
    //If DEVMODE, Run Unit Test
    #ifdef DEVMODE
        std::string test = "testItem";
        bool flag = false;
        for (uint8_t i = 0; i < size.x(); i++) {
            for (uint8_t j = 0; j < size.y(); j++) {
                storage[i][j].item_name = test;
            }
            
        }
        for (uint8_t i = 0; i < size.x(); i++) {
            for (uint8_t j = 0; j < size.y(); j++) {
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
            for (uint8_t i = 0; i < size.x(); i++) {
                for (uint8_t j = 0; j < size.y(); j++) {
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
void InventorySystem::update(Scene& scene, float dt)
{}