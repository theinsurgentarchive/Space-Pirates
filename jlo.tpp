#include "jlo.h"
#include <iostream>
#include <stdexcept>
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
template <typename T>
T* Scene::addComponent(Entity* ptr) {
    uint16_t componentId = getComponentId<T>();
            
    if (Scene::hasComponents<T>(ptr)) {
        throw std::runtime_error(
            "You cannot add the same component to this entity, "
            "check if the entity has the component prior to calling this function."
        );
    }

    if (componentId >= pools.size()) {
        pools.resize(componentId + 1);
    }
    if (pools[componentId] == nullptr) {
        pools[componentId] = std::make_unique<ComponentPool>(static_cast<uint32_t>(sizeof(T)));
    }

    void* p = pools[componentId]->get(ptr->getId());
    T* pComponent = new(p) T();
    ptr->getMask().set(componentId);
    return pComponent;
}

template <typename T>
T* Scene::getComponent(Entity* ptr) {
    int componentId = getComponentId<T>();

    if (!Scene::hasComponents<T>(ptr)) {
        return nullptr;
    }
    
    void* p = pools[componentId]->get(ptr->getId());
    return reinterpret_cast<T*>(p);
}

template <typename T>
bool checkComponents(Entity* ptr, T) {
    uint16_t cid = getComponentId<T>();
    return ptr->getMask()[cid];
}

template <typename T, typename... Ts>
bool checkComponents(Entity* ptr, T, Ts ... ts) {
    return checkComponents(ptr,T()) && checkComponents(ptr,ts...);
}
template <typename... T>
bool Scene::hasComponents(Entity* ptr) {
    return checkComponents(ptr, T()...);
}

template <typename ... T> 
std::vector<Entity*> Scene::queryEntities() {
    std::vector<Entity*> result;
    for (auto& ptr_entity : checked_out) {
        if(Scene::hasComponents<T...>(&ptr_entity)) {
            result.push_back(&ptr_entity);
        }
    }
    return result;
}