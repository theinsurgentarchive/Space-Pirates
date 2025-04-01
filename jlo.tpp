#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
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
    return {vec[0] * scale, vec[1] * scale};
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
    vec[0] += v.vec[0];
    vec[1] += v.vec[1];
    return *this;
}

namespace ecs
{
    template <typename T>
    T* ComponentManager::assign(const Entity *e_ptr)
    {
        if (e_ptr == nullptr)
            return nullptr;
        auto cid = getId<T>();
        DINFOF("component (%d) -> entity (%p | %d)\n",cid,e_ptr,e_ptr->id);
        if (e_ptr->mask.test(cid)) {
            DINFOF("reassignment for component (%d) to entity (%d), returning null\n",cid, e_ptr->id);
            return nullptr;
        }
        if (cid >= _pools.size()) {
            u32 n = _pools.size() + 1;
            DINFOF("expanded component pools to: %d -> %d\n", static_cast<u32>(_pools.size()), n);
            _pools.resize(n);
        }
        if (_pools[cid] == nullptr) {
            DINFOF("instantiated component pool (%d)\n",cid);
            _pools[cid] = std::make_unique<ComponentPool>(sizeof(T));
        }
        void *mem = _pools[cid]->get(e_ptr->id);
        DINFOF("creating new component (%d) at (%p)\n", cid, mem);
        auto *ptr_component = new (mem) T();
        e_ptr->mask.set(cid);
        return ptr_component;
    }

    template <typename T>
    void bulkAssignHelper(const Entity* e_ptr, T)
    {
        if (e_ptr == nullptr) {
            DWARN("entity pointer was null\n");
            return;
        }
        ecs::ecs.component().assign<T>(e_ptr);
    }
    template <typename T, typename... Ts>
    void bulkAssignHelper(const Entity* e_ptr, T, Ts... ts)
    {
        bulkAssignHelper(e_ptr,T());
        bulkAssignHelper(e_ptr,ts...);
    }

    template <typename... T>
    void ComponentManager::bulkAssign(const Entity* e_ptr)
    {
        bulkAssignHelper(e_ptr,T()...);
    }

    template <typename T>
    T* ComponentManager::fetch(const Entity *e_ptr)
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
    bool hasHelper(const Entity* e_ptr, T)
    {
        if (e_ptr == nullptr) {
            DWARN("entity pointer was null\n");
            return false;
        }
        uint16_t cid = getId<T>();
        return e_ptr->mask.test(cid);
    }

    template <typename T, typename... Ts>
    bool hasHelper(const Entity* e_ptr, T, Ts ... ts)
    {
        return hasHelper(e_ptr,T()) && hasHelper(e_ptr,ts...);
    }

    template <typename... T>
    bool ComponentManager::has(const Entity *e_ptr) const
    {
        return hasHelper(e_ptr,T()...);
    }      

    template <typename... T>
    void queryHelperThread(const std::vector<Entity>& entities, std::vector<const Entity*>& queried_entities, u32 start, u32 end, std::mutex& query_mutex)
    {
        std::vector<const Entity*> local_queried_entities;
        local_queried_entities.reserve(end - start);
        for (u32 i {start} ; i < end; ++i) {
            const Entity& entity = entities[i];
            if (ecs::ecs.component().has<T...>(&entity)) {
                local_queried_entities.emplace_back(&entity);
            }
        }

        std::lock_guard<std::mutex> lock(query_mutex);
        queried_entities.reserve(queried_entities.size() + local_queried_entities.size());
        for (auto& ptr : local_queried_entities) {
            queried_entities.emplace_back(ptr);
        }
    }

    template <typename... T>
    std::vector<const Entity*> ECS::query()
    {
        std::vector<const Entity*> entities;
        for (auto& ptr : _entity_manager.entities) {
            if (_component_manager.has<T...>(&ptr)) {
                DINFOF("entity (%d) matched query\n",ptr.id);
                entities.push_back(&ptr);
            }
        }
        return entities;
    }   
    
    template <typename... T>
    void System<T...>::update([[maybe_unused]] float dt)
    {
        
    }

    template <typename... T>
    System<T...>::System(
        ECS& ecs, 
        float sample_delta) 
        : 
        _ecs{ecs},
        sample_delta{sample_delta}
    {
        sample();
    }

    template <typename... T>
    time_point& System<T...>::lastSampled()
    {
        return _last_sampled;
    }

    template <typename... T>
    void System<T...>::sample()
    {
        _entities = _ecs.query<T...>();
        _last_sampled = std::chrono::high_resolution_clock::now();
    }
}

template <typename T>
void AtomicVector<T>::add(const T& obj)
{
    std::lock_guard<std::shared_mutex> lock(mutex_);
    objs_.emplace_back(obj);
}

template <typename T>
void AtomicVector<T>::add(const std::vector<T>& objs)
{
    std::lock_guard<std::shared_mutex> lock(mutex_);
    objs_.reserve(objs_.size() + objs.size());
    for (auto& obj : objs) {
        objs_.emplace_back(obj);
    }
}

template <typename T>
void AtomicVector<T>::clear()
{
    std::lock_guard<std::shared_mutex> lock(mutex_);
    objs_.clear();
}

template <typename T>
auto AtomicVector<T>::begin()
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return objs_.begin();
}

template <typename T>
auto AtomicVector<T>::end()
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return objs_.end();
}

template <typename T>
size_t AtomicVector<T>::size() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return objs_.size();
}

template <typename T>
T AtomicVector<T>::operator[] (int idx)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return objs_[idx];
}