#include "../jlo.h"
template <typename T>
T* Scene::addComponent(Entity entity) {
    int componentId = getComponentId<T>();
            
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