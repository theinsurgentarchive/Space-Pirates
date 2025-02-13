#include "../../ecs/system.h"

template <typename T>
T* EntitySystemManager::registerSystem() {
    if(EntitySystemManager::hasSystem<T>()) {
        return nullptr;
    }
    std::unique_ptr<T> p_system = std::make_unique<T>(sizeof(T));
    systems.push_back(p_system);
    return p_system.get();
}

template <typename T>
bool EntitySystemManager::hasSystem() {
    for (const auto& system : systems) {
        if (dynamic_cast<T*>(system.get()) != nullptr)
            return true;
    }
    return false;
}