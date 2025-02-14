#include "../../ecs/system.h"
#include <iostream>
template <typename T>
std::weak_ptr<T> EntitySystemManager::registerSystem() {
    // if(EntitySystemManager::hasSystem<T>()) {
    //     return nullptr;
    // }
    auto ptr = std::make_shared<T>();
    systems.push_back(ptr);
    return std::weak_ptr<T>(ptr);
}

template <typename T>
bool EntitySystemManager::hasSystem() {
    for (const auto& system : systems) {
        if (dynamic_cast<T*>(system.get()) != nullptr)
            return true;
    }
    return false;
}
