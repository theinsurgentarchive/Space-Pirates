#include "dchu.h"

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
    //Delete through all rows in the matrix
    for (int i = 0; i < inv_size[0]; i++) {
        delete [] storage[i];
    }

    //Delete Storage Matrix
    delete [] storage;
}

void Inventory::initStoreVolume(int x, int y)
{
    //Initialize Variables
    inv_size = {static_cast<uint16_t>(x),static_cast<uint16_t>(y)};
    //Initialize Inventory Matrix
    storage = new item*[inv_size[0]];
    for (int i = 0; i < inv_size[0]; i++) {
        storage[i] = new item[inv_size[1]];
    }

    //Pre-Load Item Slots
    if (inv_size[0] > 0 && inv_size[1] > 0) {
        int counter = 0;
        for (int i = 0; i < inv_size[0]; i++) {
            for (int j = 0; j < inv_size[1]; j++) {
                storage[i][j].item_name = ("Slot " + std::to_string(counter));
                storage[i][j].slot_num = ++counter;
            }
        }
    }
    //If DEVMODE, Run Unit Test
    #ifdef DEBUG
        std::string test = "testItem";
        bool flag = false;
        for (int i = 0; i < inv_size[0]; i++) {
            for (int j = 0; j < inv_size[1]; j++) {
                storage[i][j].item_name = test;
            }
            
        }
        for (int i = 0; i < inv_size[0]; i++) {
            for (int j = 0; j < inv_size[1]; j++) {
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
            for (int i = 0; i < inv_size[0]; i++) {
                for (int j = 0; j < inv_size[1]; j++) {
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
namespace ecs
{
    InventorySystem::InventorySystem()
    {

    }

    void InventorySystem::update([[maybe_unused]]float dt)
    {

    }
}
