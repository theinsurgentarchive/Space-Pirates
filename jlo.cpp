#include "jlo.h"
#include <memory>
#include <algorithm>
#include <iostream>

void show_jlo(void) {

}
//Start - Entity
Entity::Entity(EntityID i, ComponentMask m) : id{i},mask{m} {}

EntityID Entity::getId() const 
{
    return id;
}

ComponentMask& Entity::getMask() 
{
    return mask;
}
//End - Entity

//Start - Component Pool
ComponentPool::ComponentPool(uint16_t e) : element_size{e}, p_data{nullptr} 
{
    p_data = std::make_unique<char[]>(element_size);
}

ComponentPool::~ComponentPool() = default;

void* ComponentPool::get(uint16_t idx) 
{
    return p_data.get() + idx * element_size;
}
//End - Component Pool

//Start - Scene
Scene::Scene(uint16_t m) : max_entities{m} {
    for (auto i {0} ; i < m; i++) {
        free_entities.push_back({static_cast<EntityID>(i),ComponentMask()});
    }
}

Entity* Scene::createEntity() 
{
    if (Scene::getActiveEntities() >= max_entities) {
        throw new std::runtime_error("You are over the max entities");
    }
    Entity entity = std::move(free_entities.front());
    free_entities.erase(free_entities.begin());
    checked_out.push_back(std::move(entity));
    return &checked_out.back();
}

void Scene::destroyEntity(Entity* ptr) 
{
    auto it = std::find_if(checked_out.begin(),checked_out.end(),[ptr](const Entity& e) {return e.getId() == ptr->getId();});
    
    if (it != checked_out.end()) {
        checked_out.erase(it);
        free_entities.push_back(*it);
    }
}

uint16_t Scene::getActiveEntities() const
{
    return static_cast<uint16_t>(checked_out.size());
};

uint16_t Scene::getComponentCount(Entity* ptr) const
{   
    ComponentMask mask = ptr->getMask();
    return mask.count();
};
//End - Scene
