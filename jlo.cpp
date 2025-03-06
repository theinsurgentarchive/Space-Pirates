#include "jlo.h"
#include <cstdint>
#include <bitset>
#include <memory>
#include <vector>
#include <array>
#include <deque>
#include <iostream>

namespace ecs
{
    ECS ecs;
    
    Entity::Entity(eid_t i, cmask_t m) : id(i), mask(m) 
    {

    }

    ComponentPool::ComponentPool(uint16_t size) : _size {size}
    {
        _ptr_data = std::make_unique<char[]>(_size * MAX_ENTITIES);
    }

    uint16_t ComponentPool::size() const
    {
        return _size;
    }

    void *ComponentPool::get(uint16_t idx)
    {
        return _ptr_data.get() + idx * _size;
    }

    EntityManager::EntityManager(uint16_t max_entities) : _max_entities{max_entities}
    {
        for (uint16_t i{0}; i < _max_entities; i++) {
            _entities.push_back({i, cmask_t()});
            _free.push_back(i);
        }
    }

    Entity *EntityManager::get(eid_t idx)
    {
        return &_entities[idx];
    }

    Entity *EntityManager::checkout()
    {
        if (_free.empty())
            return nullptr;
        eid_t idx{_free.front()};
        _free.pop_front();
        return get(idx);
    }

    void EntityManager::ret(Entity *&e_ptr)
    {
        e_ptr->mask.reset();
        _free.push_back(e_ptr->id);
        e_ptr = nullptr;
    }

    uint16_t EntityManager::maxEntities() const
    {
        return _max_entities;
    }

    ECS::ECS() : _entity_manager{MAX_ENTITIES} 
    {

    }

    EntityManager &ECS::entity()
    {
        return _entity_manager;
    }

    ComponentManager &ECS::component()
    {
        return _component_manager;
    }
}