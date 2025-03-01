#pragma once
#include "jlo.h"

namespace ecs
{
    template <typename T>
    T* ComponentManager::assign(Entity *e_ptr)
    {
        if (e_ptr == nullptr)
            return nullptr;
        uint16_t cid = getId<T>();
        DPRINTF("component (%d) -> entity (address|id): %p | %d\n",cid,e_ptr,e_ptr->id);
        if (e_ptr->mask.test(cid))
        {
            DPRINTF("reassignment for component (%d) to entity id: %d, returning null\n",cid, e_ptr->id);
            return nullptr;
        }
        if (cid >= _pools.size())
        {
            uint16_t n = _pools.size() + 1;
            DPRINTF("component pool (%d) to: %d -> %d\n", cid, static_cast<uint16_t>(_pools.size()), n);
            _pools.resize(n);
        }
        if (_pools[cid] == nullptr) {
            DPRINTF("instantiated component pool (%d)\n",cid);
            _pools[cid] = std::make_unique<ComponentPool>(sizeof(T));
        }
        void *mem = _pools[cid]->get(e_ptr->id);
        DPRINTF("creating new component (%d) at (address): %p\n", cid, mem);
        T *ptr_component = new (mem) T();
        e_ptr->mask.set(cid);
        return ptr_component;
    }

    template <typename T>
    T* ComponentManager::fetch(Entity *e_ptr)
    {
        if (e_ptr == nullptr) {
            DPRINT("entity pointer was null");
            return nullptr;
        }
        uint16_t cid = getId<T>();
        if (!ComponentManager::has<T>(e_ptr)) {
            DPRINTF("entity (%d) does not own component (%d)",e_ptr->id,cid);
            return nullptr;
        }
        void *mem = _pools[cid]->get(e_ptr->id);
        return reinterpret_cast<T *>(mem);
    }


    template <typename T>
    bool ComponentManager::has(Entity *e_ptr)
    {
        if (e_ptr == nullptr) {
            DPRINT("entity pointer was null");
            return false;
        }
        uint16_t cid = getId<T>();
        return e_ptr->mask.test(cid);
    }      
}
