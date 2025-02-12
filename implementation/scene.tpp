#include "../jlo.h"
#include <stdexcept>
#include <iostream>

template <typename T>
T* Scene::addComponent(Entity& entity) {
    int componentId = getComponentId<T>();
            
    if (Scene::hasComponent<T>(entity)) {
        std::cout << "here";
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

    void* p = pools[componentId]->get(entity.getId());
    T* pComponent = new(p) T();
    entity.getMask().set(componentId);
    return pComponent;
}

template <typename T>
T* Scene::getComponent(Entity& entity) {
    int componentId = getComponentId<T>();

    if (!Scene::hasComponent<T>(entity)) {
        return nullptr;
    }
    
    void* p = pools[componentId]->get(entity.getId());
    return reinterpret_cast<T*>(p);
}

template <typename T>
bool Scene::hasComponent(Entity& entity) {
    int componentId = getComponentId<T>();
    return entity.getMask().test(componentId);
}