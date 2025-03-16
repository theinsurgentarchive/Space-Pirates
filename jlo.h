#pragma once
#include <chrono>
#include <cstdint>
#include <bitset>
#include <memory>
#include <vector>
#include <deque>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <GL/glx.h>

#include "fonts.h"

#define MAX_COMPONENTS 32
#define MAX_ENTITIES 20001

#define TRANSFORM ecs::Transform
#define SPRITE ecs::Sprite
#define PHYSICS ecs::Physics
#define HEALTH ecs::Health

#define _RESET "\033[0m"
#define _RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
#define _INFO(fmt) \
    _RGB(102, 204, 255) "INFO: " _RESET _RGB(255, 255, 255) fmt _RESET
#define _WARN(str) \
    _RGB(255,204,0) "WARNING: " _RESET _RGB(255,255,255) str _RESET
#define _ERROR(str) \
    _RGB(255,76,76) "ERROR: " _RESET _RGB(255,255,255) str _RESET
#ifdef DEBUG
#include <iostream>
#include <cstdio>
#define DINFOF(fmt, ...) printf(_INFO(fmt), __VA_ARGS__),fflush(stdout)
#define DWARNF(fmt, ...) printf(_WARN(fmt), __VA_ARGS__),fflush(stdout)
#define DERRORF(fmt, ...) printf(_ERROR(fmt), __VA_ARGS__),fflush(stdout),exit(1)
#define DINFO(str) std::cout << _INFO(str) << std::flush
#define DWARN(str) std::cout << _WARN(str) << std::flush
#define DERROR(str) std::cout << _ERROR(str) << std::flush,exit(1)
#else
#define DINFOF(fmt, ...)
#define DWARNF(fmt, ...)
#define DERRORF(fmt, ...)
#define DINFO(str)
#define DWARN(str)
#define DERROR(str)
#endif

void show_jlo(Rect* r);

struct SpriteSheet;
struct Camera;
class World;
struct Texture;
class TextureLoader;
class Animation;
class AnimationBuilder;

template <typename T>
class Vec2;

namespace wfc
{
    struct TileMeta;
    class TileBuilder;
    struct Cell;
    class Grid;
    class TilePriorityQueue;
    class WaveFunction;
}

namespace ecs
{
    struct Planet;
    struct Physics;
    struct Transform;
    struct Health;
    struct Sprite;
    struct Entity;
    class ComponentPool;
    class ComponentManager;
    class EntityManager;
    class ECS;
    template <typename... T>
    class System;
    class PhysicsSystem;
    class RenderSystem;
}

extern uint16_t counter;
extern std::shared_ptr<Texture> load_tex(const std::string&, bool);
template <class T>
uint16_t getId()
{
    static int cid = counter++;
    return cid;
}

using u16 = uint16_t;
using i32 = int32_t;
using v2u = Vec2<u16>;
using v2i = Vec2<i32>;
using v2f = Vec2<float>;
using time_point = std::chrono::high_resolution_clock::time_point;
typedef uint16_t eid_t;
typedef std::bitset<MAX_COMPONENTS> cmask_t;

enum Direction
{
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};

template <typename T>
class Vec2
{
    public:
        T vec[2];
        Vec2();
        Vec2(T x, T y);
        Vec2<T> operator-() const;
        Vec2<T> operator+(const Vec2<T>& v) const;
        Vec2<T> operator*(float scale) const;
        T operator[](int idx) const;
        T& operator[](int idx);
        Vec2<T>& operator+=(const Vec2<T>& v);
};


struct Camera
{
    v2f pos;
    v2u dim;
    Camera(v2f pos, v2u dim);
    void move(v2f delta);
    void update() const;
};


struct Texture
{
    const v2u dim;
    bool alpha {false};
    std::shared_ptr<GLuint> tex;
    Texture(const v2u& dim, bool alpha);
};

struct SpriteSheet
{
    const v2u frame_dim;
    const v2u sprite_dim;
    std::shared_ptr<Texture> tex;
    SpriteSheet(
        const v2u& frame_dim, 
        const v2u& sprite_dim, 
        const std::shared_ptr<Texture> tex);
    void render(uint16_t frame, v2f pos);
    void render(uint16_t frame, v2f pos, v2f scale);
};

class World
{
    private:
        std::vector<std::vector<ecs::Entity*>> _grid;
        std::unordered_map<std::string,wfc::TileMeta> _tiles;
    public:
        std::vector<std::vector<ecs::Entity*>>& tiles();
        ~World();
        World(
            const v2f& origin, wfc::Grid& grid,
            std::unordered_map<std::string,wfc::TileMeta>& tiles);
};

namespace wfc
{
    struct TileMeta
    {
        float weight;
        std::string ssheet;
        std::array<std::unordered_set<std::string>, 4> rules;
        std::unordered_map<std::string,float> coefficients;
        TileMeta(
            float weight, 
            std::string ssheet,
            std::array<std::unordered_set<std::string>,4>& rules, 
            std::unordered_map<std::string,
            float>& coefficients);
    };

    class TileBuilder
    {
        private:
            float _weight;
            std::string _ssheet;
            std::array<std::unordered_set<std::string>,4> _rules;
            std::unordered_map<std::string,float> _coefficients;
        public:
            TileBuilder(float weight, const std::string& ssheet);
            TileBuilder& rule(
                int dir, 
                const std::string& tile);
            TileBuilder& omni(const std::string& tile);
            TileBuilder& coefficient(
                const std::string& tile, 
                float weight);
            TileMeta build();
    };

    struct Cell 
    {
        v2i pos;
        std::unordered_set<std::string> states;
        std::string state;
        Cell(
            const v2i& pos, 
            const std::unordered_set<std::string>& states);
        uint16_t entropy() const;
        bool collapsed() const;
    };

    class Grid 
    {
        private:
            std::vector<std::vector<Cell>> _cells;
            v2u _size;
        public:
            Grid(
                v2u size, 
                const std::unordered_set<std::string>& states);
            v2u size() const;
            Cell* get(v2i pos);
            std::vector<std::vector<Cell>>& cells();
            bool collapsed();
            void print();
    };

    class TilePriorityQueue
    {
        private:
            std::vector<Cell*> _queue;
            void _swap(uint16_t i1, uint16_t i2);
            void _bubbleUp(uint16_t idx);
            void _bubbleDown(uint16_t idx);
        public:
            TilePriorityQueue(Grid& grid);
            void insert(Cell* cell);
            bool empty();
            Cell* pop();
    };

    class WaveFunction 
    {
        private:
            Grid& _grid;
            std::unordered_map<std::string,TileMeta>& _tiles;
            TilePriorityQueue _queue;
            float _calculateTileWeight(const v2i& pos, const TileMeta& meta);
            void _collapse(Cell* c);
            void _propagate(Cell* c);
        public:
            WaveFunction(
                Grid& grid, 
                std::unordered_map<std::string,TileMeta>& tiles);
            void run();
    };
}

namespace ecs
{
    class ECS;
    extern ECS ecs;

    struct Planet
    {
        float temperature;
    };

    struct Physics
    {
        v2f vel;
        v2f acc;
        float mass;
        bool enabled {true};
    };

    struct Transform
    {
        v2f pos;
        v2f scale {1,1};
        float rotation;
    };

    struct Health
    {
        float health;
        float max;
    };

    struct Sprite
    {
        std::string ssheet;
        u16 frame;
        u16 render_order {0};
        bool invertY {false};
    };

    struct Entity
    {
        eid_t id;
        mutable cmask_t mask;
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

            /*
                Assigns component <T> to the entity,
                if the entity has already been assigned <T>, then
                this function will return nullptr.

                @returns a pointer to <T>
            */
            template <typename T>
            T* assign(const Entity* e_ptr);
            
            template <typename... T>
            void bulkAssign(const Entity* e_ptr);

            /*
                Retrieves a component <T> assigned to the entity;
                if the entity has not been assigned with <T>
                or the entity is nullptr,
                this function will return nullptr.

                @returns a pointer to <T>
            */
            template <typename T>
            T* fetch(const Entity* e_ptr);

            /*
                Checks whether the entity has 
                been assigned all components <T...>,

                @returns a bool of whether the entity has all components
            */
            template <typename... T>
            bool has(const Entity* e_ptr) const;
    };

    class EntityManager
    {
        /*
            Implementation notes:
                1. All entities are initialized and statically defined 
                at startup,
                meaning that if 'MAX_ENTITIES = 500', 
                MAX_ENTITIES == entities.size()
                after the constructor is called.
        */
        private:
            uint16_t _max_entities;
            std::deque<eid_t> _free;
        public:
            std::vector<Entity> entities;
            EntityManager(uint16_t max_entities);

            /*
                Checks out an entity from the system.

                This prevents the system from clearing the mask
                or data on the heap prior to calling ret().
            */
            Entity* checkout();

            /*
                Returns the entity back to the system, 
                allowing the id/bitset
                to be reused.

                e.g.
                Entity ID = 1,
                Mask = 0...010, (holds the components for entity...
                ) assume the size of mask is equal to 'MAX_COMPONENTS'

                The entity with ID 1 is currently assigned 
                the second component.

                After calling ret,

                Entity ID = 1;
                Mask = 0...000
            */
            void ret(Entity* e_ptr);
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
            template <typename... T>
            std::vector<const Entity*> query() const;
    };

    template <typename... T>
    class System
    {
        private:
            time_point _last_sampled;
        protected:
            ECS& _ecs;
            std::vector<const Entity*> _entities;
        public:
            const float sample_delta;
            System(ECS& ecs, float sample_delta);
            virtual void update(float dt);
            time_point& lastSampled();
            void sample();
    };

    class PhysicsSystem : public System<Transform,Physics>
    {
        public:
            PhysicsSystem(ECS& ecs, float sample_delta);
            void update(float dt) override;
    };

    class RenderSystem : public System<Transform,Sprite>
    {
        public:
            RenderSystem(ECS& ecs, float sample_delta);
            void update(float dt) override;
    };
}
#include "jlo.tpp"
