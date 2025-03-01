#include "jlo.h"
#include <cstdint>
#include <bitset>
#include <memory>
#include <vector>
#include <array>
#include <deque>
#include <iostream>

Vec2f::Vec2f() : v{0, 0} 
{

}

Vec2f::Vec2f(float x, float y) : v{x, y} 
{

}

float Vec2f::getX() const
{
    return v[0];
}

float Vec2f::getY() const
{
    return v[1];
}

void Vec2f::setX(float x)
{
    v[0] = x;
}

void Vec2f::setY(float y)
{
    v[1] = y;
}

Vec2f Vec2f::operator-() const
{
    return Vec2f(-v[0], -v[1]);
}

Vec2f Vec2f::operator+(Vec2f vec) const
{
    return Vec2f(v[0] + vec.v[0], v[1] + vec.v[1]);
}

Vec2f Vec2f::operator*(float scale) const
{
    return Vec2f(getX() * scale, getY() * scale);
}

float Vec2f::operator[](int idx) const
{
    return v[idx];
}

float &Vec2f::operator[](int idx)
{
    return v[idx];
}

Vec2f &Vec2f::operator+=(const Vec2f &vec)
{
    v[0] += vec.v[0];
    v[1] += vec.v[1];
    return *this;
}

Vec2i::Vec2i() : v{0, 0} 
{

}

Vec2i::Vec2i(int x, int y) : v{x, y} 
{

}

int Vec2i::getX() const
{
    return v[0];
}

int Vec2i::getY() const
{
    return v[1];
}

void Vec2i::setX(int x)
{
    v[0] = x;
}

void Vec2i::setY(int y)
{
    v[1] = y;
}

Vec2i Vec2i::operator-() const
{
    return Vec2i(-v[0], -v[1]);
}

Vec2i Vec2i::operator+(Vec2i vec) const
{
    return Vec2i(v[0] + vec.v[0], v[1] + vec.v[1]);
}

Vec2i Vec2i::operator*(float scale) const
{
    return Vec2i(getX() * scale, getY() * scale);
}

int Vec2i::operator[](int idx) const
{
    return v[idx];
}

int &Vec2i::operator[](int idx)
{
    return v[idx];
}

Vec2i &Vec2i::operator+=(const Vec2i &vec)
{
    v[0] += vec.v[0];
    v[1] += vec.v[1];
    return *this;
}

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