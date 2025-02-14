#include "../../ecs/system.h"
#include <iostream>
template <typename T>
std::weak_ptr<T> EntitySystemManager::registerSystem() {
    // if(EntitySystemManager::hasSystem<T>()) {
    //     return nullptr;
    // }

    auto p_system = std::make_shared<T>();  // No arguments, using default constructor
    // Move the unique_ptr into the vector
    systems.push_back(p_system);
    return std::weak_ptr<T>(p_system);
}

template <typename T>
bool EntitySystemManager::hasSystem() {
    for (const auto& system : systems) {
        if (dynamic_cast<T*>(system.get()) != nullptr)
            return true;
    }
    return false;
}