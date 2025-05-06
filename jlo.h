#pragma once
#include <chrono>
#include <cstdint>
#include <bitset>
#include <memory>
#include <vector>
#include <queue>
#include <deque>
#include <random>
#include <array>
#include <shared_mutex>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <GL/glx.h>
#include "fonts.h"

#define _MAX_COMPONENTS 32
#define MAX_ENTITIES 62501
#define _MAX_TEMPERATURE 100.0f
#define _MIN_TEMPERATURE -30.0f

#define TRANSFORM ecs::Transform
#define SPRITE ecs::Sprite
#define NAME ecs::Name
#define PHYSICS ecs::Physics
#define HEALTH ecs::Health
#define COLLIDER ecs::Collider

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

enum BiomeType
{
    FOREST,
    DESERT,
    TAIGA,
    MEADOW,
    HELL
};

enum LootType
{
    OXYGEN,
    FUEL
};

enum Direction
{
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};

struct Biome;
struct SpriteSheet;
class Camera;
struct World;
class Loot;
class LootSelector;
struct Texture;
struct Collision;
template <typename T>
class AtomicVector;
class ThreadPool;

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
    struct Nameable;
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

        T dot(const Vec2<T>& other) const {
            return vec[0] * other.vec[0] + vec[1] * other.vec[1];
        }
};

using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i32 = int32_t;
using v2u = Vec2<u16>;
using v2i = Vec2<i32>;
using v2f = Vec2<float>;
using cmask = std::bitset<_MAX_COMPONENTS>;
using time_point = std::chrono::high_resolution_clock::time_point;

void checkRequiredSprites();

void show_jlo(Rect* r);
extern u16 counter;
extern void loadTextures(
    std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets);
extern std::shared_ptr<Texture> loadTexture(const std::string&, bool);
extern void collisions(const Camera& camera, ThreadPool& pool);
template <class T>
u16 getId()
{
    static int cid = counter++;
    return cid;
}

struct Loot
{
    LootType type;
    float amount;
    float weight;
    Loot(LootType type, float amount, float weight);
};

class LootSelector
{
    public:
        Loot random(std::mt19937 rand);
        LootSelector& addLoot(std::initializer_list<Loot> loot);
    private:
        std::vector<Loot> loot_;
};


struct Biome
{
    const BiomeType type;
    const v2f temperature;
    const v2f humidity;
    const std::string description;
    Biome(
        BiomeType type, 
        const v2f& temperature, 
        const v2f& humidity, 
        const std::string& description);
    std::unordered_map<std::string,wfc::TileMeta> tiles();
    static std::vector<std::string> decor(BiomeType type);
};

/*
    Camera of the game, center of the camera is bound to 'pos_'.
*/
class Camera
{
    public:
        void bind(v2f& pos);
        void bind(const ecs::Entity* entity);
        bool visible(v2f curr) const;
        void update() const;
        v2u dim() const;
       
        /*

                Finds entities that are currently visible to the camera
            and places them in 'visible_entities'
        
            @return collection of entities        
        */
        std::vector<const ecs::Entity*> findVisible(
            std::vector<const ecs::Entity*>& entities,
            ThreadPool& pool) const;
        Camera(v2f& pos, v2u& dim);
    private:
        std::reference_wrapper<v2f> pos_;
        std::reference_wrapper<v2u> dim_;
        void visibleHelper(
            std::vector<const ecs::Entity*>& entities, 
            std::vector<const ecs::Entity*>& visible_entities,
            const u32 start, 
            const u32 end,
            std::mutex& visible_entities_mutex) const;
};


struct Texture
{
    const v2u dim;
    bool alpha {false};
    std::unique_ptr<GLuint> tex;
    Texture(const v2u& dim, bool alpha);
};

struct SpriteSheet
{
    const v2u frame_dim;
    const v2u sprite_dim;
    std::shared_ptr<Texture> tex;
    bool animated;
    SpriteSheet(
        const v2u& frame_dim, 
        const v2u& sprite_dim, 
        const std::shared_ptr<Texture> tex,
        bool animated=false);
    u16 maxFrames() const;
    void render(u16 frame, v2f pos, v2f scale, bool invertY);
};

class SpriteSheetLoader
{
    public:
        SpriteSheetLoader(std::unordered_map<
            std::string,
            std::shared_ptr<SpriteSheet>>& ssheets);
        SpriteSheetLoader& loadStatic(
            const std::string& key,
            const std::shared_ptr<Texture>& tex,
            const v2u& frame_dim = v2u{1,1},
            const v2u& sprite_dim = v2u{0,0},
            bool animated=false
        );
    private:
        std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets_;
};

struct WorldGenerationSettings
{
    v2f origin;
    float temperature, humidity;
    u16 radius;
    u32 biome_seed;
    WorldGenerationSettings(float temperature, float humidity, u16 radius, u32 seed);
};

struct World
{
    using WorldCell = std::vector<const ecs::Entity*>;
    std::vector<std::vector<WorldCell>> cells;
    World(WorldGenerationSettings settings);
    WorldCell& cellWithMaxArea();
    ~World();
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
            TileBuilder& omni(std::initializer_list<std::string> tiles);
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
        u16 entropy() const;
        bool collapsed() const;
    };

    struct Grid
    {
        const v2u size;
        std::vector<Cell> cells;
        Grid(const v2u& size);
        bool collapsed();
        Cell* get(v2i pos);
    };

    class TilePriorityQueue
    {
        private:
            std::vector<Cell*> _queue;
            void _swap(u16 i1, u16 i2);
            void _bubbleUp(u16 idx);
            void _bubbleDown(u16 idx);
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

    struct Name
    {
        int alignment {0};
        u32 cref {0x00ffffff};
        v2i offset {0,0};
        std::string name;
    };

    struct Collider
    {
        bool passable {true};
        v2f offset;
        v2u dim;
        std::function<void(const ecs::Entity*, const ecs::Entity*)> callback;
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
        bool invert_y {false};
    };

    struct Entity
    {
        u32 id;
        mutable cmask mask;
        Entity(u32 i, cmask m);
    };

    class ComponentPool
    {
        private:
            std::unique_ptr<char[]> _ptr_data;
            u32 _size;
        public:
            ComponentPool(u32 size);
            u32 size() const;
            void* get(u16 idx);
    };

    class ComponentManager
    {
        public:
            /*
                Assigns component <T> to the entity,
                if the entity has already been assigned <T>, then
                this function will return nullptr.

                @returns a tuple of T*
            */
            
            template <typename... T>
            auto assign(const Entity* e_ptr);

            /*
                Retrieves a component <T> assigned to the entity;
                if the entity has not been assigned with <T>
                or the entity is nullptr,
                this function will return nullptr.

                @returns a tuple of T*
            */
            template <typename... T>
            auto fetch(const Entity* e_ptr);

            /*
                Checks whether the entity has 
                been assigned all components <T...>,

                @returns a bool of whether the entity has all components
            */
            template <typename... T>
            bool has(const Entity* e_ptr);
        private:
            std::vector<std::unique_ptr<ComponentPool>> _pools;
            std::shared_mutex component_mutex_;
            template <typename T>
            std::tuple<T*> fetchHelper(const Entity* entity, std::vector<std::unique_ptr<ComponentPool>>& pools, T);
            template <typename T, typename... Ts>
            auto fetchHelper(const Entity* entity, std::vector<std::unique_ptr<ComponentPool>>& pools, T, Ts... ts);
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
            u32 _max_entities;
            std::deque<u32> _free;
        public:
            std::vector<Entity> entities;
            EntityManager(u32 max_entities);

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
            void ret(const Entity* e_ptr);
            u32 maxEntities() const;
            bool isFree(const Entity* e_ptr);
            
    };

    class ECS
    {
        public:
            ECS();
            EntityManager& entity();
            ComponentManager& component();
            template <typename... T>
            std::vector<const Entity*> query();
        private:
            std::mutex _entity_manager_mutex;
            EntityManager _entity_manager;
            ComponentManager _component_manager;
    };

    template <typename... T>
    class System
    {
        private:
            time_point _last_sampled;
        protected:
            ECS& _ecs;
            //std::vector<const Entity*> _entities; //billie for custom sample
        public:
            std::vector<const Entity*> _entities;  
            const float sample_delta;
            System(ECS& ecs, float sample_delta);
            virtual void update(float dt);
            time_point& lastSampled();
            virtual void sample();
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
            void sample() override;
    };
}

/*
        Thread-safe wrapper for std::vector, the usage is generally the same. 
    All write operations are thread-safe and have synchronization primitives to 
    prevent multiple writers, but permit concurrent readers 
    using std::shared_mutex. This is an implementation detail.

    e.x: AtomicVector<int> intAtomicVector;

    intAtomicVector.add(1); //adding an integer
    intAtomicVector.add({1,1}); //adding a vector list

        This wrapper also supports a range-based for loop e.x:

    for (auto& obj : intAtomicVector)
        std::cout << obj << '\n';
*/

template <typename T>
class AtomicVector {
    public:
        void add(const T& obj);
        void add(const std::vector<T>& objs);
        void clear();
        auto begin();
        auto end();
        size_t size() const;
        T operator[] (int idx) const;
        void set(const T& obj, int idx);
    private:
        std::shared_mutex mutex_;
        std::vector<T> objs_;
};

/*
        A basic thread pool that allows for concurrent task scheduling with 
    'nthreads' worker threads. This limits the overhead of creating multiple
    new threads for each task.
    
    e.x: ThreadPool pool {4}; //spawns a pool with 4 threads

    pool.enqueue([](){ * your task * });
*/

class ThreadPool
{
    public:
        ThreadPool(u32 nthreads);
        ~ThreadPool();
        void enqueue(std::function<void()> task);
        u32 size() const;
    private:
        std::mutex queue_mutex_;
        std::vector<std::thread> workers_;
        std::atomic<bool> stop_;
        std::queue<std::function<void()>> task_queue_;
        std::condition_variable task_available_;
        u32 size_;
        void workerThread();
};

struct Collision
{
    const ecs::Entity *a, *b;
    Collision(const ecs::Entity* a, const ecs::Entity* b);
};

class CollisionHandler
{
    public:
        void addHandler(std::function<void()> handler);
        void handle(std::vector<Collision> collisions);
    private:
        std::vector<std::function<void()>> handlers_;
};

class PlayerFactory
{
    public:
        const ecs::Entity* createPlayer();
        PlayerFactory(Camera& camera);
    private:
        Camera& camera_;
};

bool collided(TRANSFORM*, TRANSFORM*, COLLIDER*, COLLIDER*);
#include "jlo.tpp"
