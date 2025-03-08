
#include "jsandoval.h"
#include "jlo.h"
#include <iostream>
namespace ecs {
    extern ECS ecs;
Entity* character_x()
{
    
    auto x = ecs::ecs.entity().checkout();
    auto health = ecs::ecs.component().assign<HEALTH>(x);
    auto transform = ecs::ecs.component().assign<TRANSFORM>(x);
    
    if (health) {
        health -> health = 50;
        health -> max = 100;
    }
    //bring in xres yres to file
    if (transform) {
        transform -> pos[0] = 20;
        transform -> pos[1] = 30;
    }

    auto retrievedHealth = ecs::ecs.component().fetch<HEALTH>(x);
    auto retrievedPos = ecs::ecs.component().fetch<TRANSFORM>(x);
    if (retrievedPos) {
        std::cout << "Position on x-axis: " << retrievedPos -> pos[0] << " y Position " << retrievedPos -> pos[1] << std::endl;
    }
    else {
        std::cout << "Failed to retrieve Health Component." <<std::endl;
    }
    if (retrievedHealth) {
        std::cout << "Health: " << retrievedHealth -> health << std::endl;
        std::cout << "Max Health: " << retrievedHealth -> max << std::endl;
    }
    else {
        std::cout << "Failed to retrieve Health Component." <<std::endl;
    }
    return x;
}
}