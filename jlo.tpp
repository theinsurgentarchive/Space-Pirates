#pragma once
#include <iostream>
#include "jlo.h"

template <typename T>
Vec2<T>::Vec2() : vec{T(0),T(0)} {}

template <typename T>
Vec2<T>::Vec2(T x, T y) : vec{x,y} {}

template <typename T>
Vec2<T> Vec2<T>::operator-() const
{
    return Vec2<T>(-vec[0],-vec[1]);
}

template <typename T>
Vec2<T> Vec2<T>::operator+(const Vec2<T>& v) const
{
    return Vec2<T>(vec[0] + v.vec[0], vec[1] + v.vec[1]);
}

template <typename T>
Vec2<T> Vec2<T>::operator*(float scale) const
{
    return Vec2<T>(vec[0] * scale, vec[1] * scale);
}

template <typename T>
T Vec2<T>::operator[](int idx) const
{
    return vec[idx];
}

template <typename T>
T& Vec2<T>::operator[](int idx)
{
    return vec[idx];
}

template <typename T>
Vec2<T>& Vec2<T>::operator+=(const Vec2<T>& v)
{
    return vec[0] += v.vec[0], vec[1] += v.vec[1], *this;
}
namespace ecs
{
    template <typename T>
    T* ComponentManager::assign(Entity *e_ptr)
    {
        if (e_ptr == nullptr)
            return nullptr;
        uint16_t cid = getId<T>();
        DINFOF("component (%d) -> entity (%p | %d)\n",cid,e_ptr,e_ptr->id);
        if (e_ptr->mask.test(cid)) {
            DINFOF("reassignment for component (%d) to entity (%d), returning null\n",cid, e_ptr->id);
            return nullptr;
        }
        if (cid >= _pools.size()) {
            uint16_t n = _pools.size() + 1;
            DINFOF("component pool (%d) to: %d -> %d\n", cid, static_cast<uint16_t>(_pools.size()), n);
            _pools.resize(n);
        }
        if (_pools[cid] == nullptr) {
            DINFOF("instantiated component pool (%d)\n",cid);
            _pools[cid] = std::make_unique<ComponentPool>(sizeof(T));
        }
        void *mem = _pools[cid]->get(e_ptr->id);
        DINFOF("creating new component (%d) at (%p)\n", cid, mem);
        T *ptr_component = new (mem) T();
        e_ptr->mask.set(cid);
        return ptr_component;
    }

    template <typename T>
    T* ComponentManager::fetch(Entity *e_ptr)
    {
        if (e_ptr == nullptr) {
            DWARN("entity pointer was null\n");
            return nullptr;
        }
        uint16_t cid = getId<T>();
        if (!ComponentManager::has<T>(e_ptr)) {
            DINFOF("entity (%d) does not own component (%d)\n",e_ptr->id,cid);
            return nullptr;
        }
        void *mem = _pools[cid]->get(e_ptr->id);
        return reinterpret_cast<T *>(mem);
    }


    template <typename T>
    bool hasHelper(Entity* e_ptr, T)
    {
        if (e_ptr == nullptr) {
            DWARN("entity pointer was null");
            return false;
        }
        uint16_t cid = getId<T>();
        return e_ptr->mask.test(cid);
    }

    template <typename T, typename... Ts>
    bool hasHelper(Entity* e_ptr, T, Ts ... ts)
    {
        return hasHelper(e_ptr,T()) && hasHelper(e_ptr,ts...);
    }

    template <typename... T>
    bool ComponentManager::has(Entity *e_ptr)
    {
        return hasHelper(e_ptr,T()...);
    }      

    template <typename... T>
    std::vector<Entity*> ECS::query()
    {
        std::vector<Entity*> entities;
        for (auto& ptr : _entity_manager.entities) {
            if (_component_manager.has<T...>(&ptr)) {
                entities.push_back(&ptr);
            }
        }
        return entities;
    }     

    template <typename T>
    void SystemManager::registerSystem()
    {
        auto ptr = std::make_shared<T>();
        _systems.push_back(ptr);
    }
}
