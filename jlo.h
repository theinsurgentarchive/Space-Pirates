#pragma once
#include <cstdint>
#include <bitset>
#include <memory>
#include <vector>
#include <deque>
#include <unordered_map>
#define MAX_COMPONENTS 32
#define MAX_ENTITIES 500

#define TRANSFORM ecs::Transform
#define SPRITE ecs::Sprite
#define PHYSICS ecs::Physics
#define HEALTH ecs::Health

#ifdef DEBUG
#include <iostream>
#include <cstdio>
#define DPRINTF(fmt, ...) printf(fmt, __VA_ARGS__)
#define DPRINT(str) std::cout << str << std::flush
#else
#define DPRINTF(fmt, ...)
#define DPRINT(str)
#endif

extern uint16_t counter;

template <class T>
uint16_t getId()
{
    static int cid = counter++;
    return cid;
}

typedef uint16_t eid_t;
typedef std::bitset<MAX_COMPONENTS> cmask_t;

template <typename T>
class Vec2
{
    public:
        T vec[2];
        Vec2();
        Vec2(T x, T y);
        Vec2<T> operator- () const;
        Vec2<T> operator+ (const Vec2<T>& v) const;
        Vec2<T> operator* (float scale) const;
        T operator[] (int idx) const;
        T& operator[] (int idx);
        Vec2<T>& operator+= (const Vec2<T>& v);
};

namespace ecs 
{

    class ECS;
    extern ECS ecs;

    struct Physics
    {
        Vec2<float> vel;
        Vec2<float> acc;
        float mass;
    };

    struct Transform 
    {
        Vec2<float> pos;
        Vec2<float> scale;
        float rotation;
    };

    struct Health 
    {
        float health;
        float max;
    };

    struct Sprite
    {
        char* texture {nullptr};
    };

    struct Entity 
    {
        eid_t id;
        cmask_t mask;
        Entity(eid_t i, cmask_t m);
    };

    class ComponentPool 
    {
        private:
            std::unique_ptr<char[]> _ptr_data;
            uint16_t _size;
        public:
            ComponentPool(uint16_t s);
            uint16_t size() const;
            void* get(uint16_t idx);
    }; 

    class ComponentManager
    {
        private:
            std::vector<std::unique_ptr<ComponentPool>> _pools;
        public:
            template <typename T>
            T* assign(Entity* e_ptr);

            template <typename T>
            T* fetch(Entity* e_ptr);

            template <typename T>
            bool has(Entity* e_ptr);
    };

    class EntityManager
    {
        private:
            uint16_t _max_entities;
            std::vector<Entity> _entities;
            std::deque<eid_t> _free;
        public:
            EntityManager(uint16_t max_entities);
            Entity* get(eid_t idx);
            Entity* checkout();
            void ret(Entity*& e_ptr);
            uint16_t maxEntities() const;
    };

    class ECS
    {
        private:
            EntityManager _entity_manager;
            ComponentManager _component_manager;
        public:
            ECS();
            EntityManager& entity();
            ComponentManager& component();
    };

    class System
    {

    };
}
#include "jlo.tpp"