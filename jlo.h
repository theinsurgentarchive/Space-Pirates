#pragma once
#include "fonts.h"
#include <cstdint>
#include <bitset>
#include <memory>
#include <vector>
#include <deque>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <GL/glx.h>

#define MAX_COMPONENTS 32
#define MAX_ENTITIES 500

#define TRANSFORM ecs::Transform
#define SPRITE ecs::Sprite
#define PHYSICS ecs::Physics
#define HEALTH ecs::Health

#define _RESET "\033[0m"
#define _RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
#define _INFO(str) _RGB(102, 204, 255) "INFO: " _RESET _RGB(255,255,255) str _RESET
#define _WARN(str) _RGB(255,204,0) "WARNING: " _RESET _RGB(255,255,255) str _RESET
#define _ERROR(str) _RGB(255,76,76) "ERROR: " _RESET _RGB(255,255,255) str _RESET
#ifdef DEBUG
#include <iostream>
#include <cstdio>
#define DINFOF(fmt, ...) printf(_INFO(fmt), __VA_ARGS__)
#define DWARNF(fmt, ...) printf(_WARN(fmt), __VA_ARGS__)
#define DERRORF(fmt, ...) printf(_ERROR(fmt), __VA_ARGS__)
#define DINFO(str) std::cout << _INFO(str) << std::flush
#define DWARN(str) std::cout << _WARN(str) << std::flush
#define DERROR(str) std::cout << _ERROR(str) << std::flush
#else
#define DINFOF(fmt, ...)
#define DWARNF(fmt, ...)
#define DERRORF(fmt, ...)
#define DINFO(str)
#define DWARN(str)
#define DERORR(str)
#endif

void show_jlo(Rect* r);

struct Texture;
class TextureLoader;
class Animation;
class AnimationBuilder;

template <typename T>
class Vec2;

namespace wfc
{
    struct Tile;
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
    class System;
    class PhysicsSystem;
    class RenderSystem;
}

extern uint16_t counter;
extern TextureLoader tl;
template <class T>
uint16_t getId()
{
    static int cid = counter++;
    return cid;
}

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


struct Texture
{
    Vec2<uint16_t> dim;
    bool alpha {false};
    std::shared_ptr<GLuint> tex;
    Texture(const Vec2<uint16_t>& dim, bool alpha);
};

class TextureLoader
{
    public:
        std::shared_ptr<Texture> load(const std::string& file_name, bool alpha);
};

class Animation
{
    private:
        std::string _texture_key;
        Vec2<uint16_t> _sprite_dim;
        Vec2<uint16_t> _frame_dim;
        std::array<Vec2<uint16_t>,2> _frame_range;
        uint16_t _frame {0};
    public:
        Animation(const std::string& texture_key, 
            const Vec2<uint16_t>& sprite_dim, 
            const Vec2<uint16_t>& frame_dim, 
            const std::array<Vec2<uint16_t>,2>& frame_range);
        Animation& operator+(int value);
        Animation& operator-(int value);
        Animation& operator++();
        Animation& operator=(uint16_t frame);
        uint16_t getFrame() const;
        uint16_t getMaxFrames() const;
        std::string getTextureKey() const;
        Vec2<uint16_t> getSpriteDim() const;
        Vec2<uint16_t> getFrameDim() const;
};

class AnimationBuilder
{
    private:
        std::string _texture_key;
        Vec2<uint16_t> _sprite_dim;
        Vec2<uint16_t> _frame_dim;
        std::array<Vec2<uint16_t>,2> _frame_range;
    public:
        AnimationBuilder& setTextureKey(const std::string& texture_key);
        AnimationBuilder& setSpriteDimension(const Vec2<uint16_t>& sprite_dim);
        AnimationBuilder& setFrameDimension(const Vec2<uint16_t>& frame_dim);
        AnimationBuilder& setFrameRange(const std::array<Vec2<uint16_t>,2>& frame_range);
        std::shared_ptr<Animation> build();
};

namespace wfc
{
    struct Planet
    {

    };

    struct Tile 
    {
        float weight;
        std::array<std::vector<std::string>, 4> rules;
        std::unordered_map<std::string,float> coefficients;
        Tile(float weight, std::array<std::vector<std::string>,4>& rules, std::unordered_map<std::string,float>& coefficients);
    };

    class TileBuilder
    {
        private:
            float _weight;
            std::array<std::vector<std::string>,4> _rules;
            std::unordered_map<std::string,float> _coefficients;
        public:
            TileBuilder& setWeight(float weight);
            TileBuilder& addRule(const Direction& dir, const std::string& tile);
            TileBuilder& addCoefficient(const std::string& tile, float weight);
            Tile build();
    };

    struct Cell 
    {
        std::vector<std::string> states;
        Vec2<int32_t> pos;
        Cell(std::vector<std::string> states, Vec2<int32_t> pos);
        uint16_t entropy() const;
    };

    class Grid 
    {
        private:
            std::vector<std::vector<std::string>> _cells;
            Vec2<uint16_t> _size;
        public:
            Grid(Vec2<uint16_t> size);
            Vec2<uint16_t> size() const;
            std::string get(Vec2<int32_t> pos) const;
            void set(Vec2<int32_t> pos, std::string name);
            bool collapsed(const Vec2<int32_t>& pos);
            void print();
    };

    class TilePriorityQueue
    {
        private:
            std::vector<Cell> _queue;
            void _swap(uint16_t i1, uint16_t i2);
            void _bubbleUp(uint16_t idx);
            void _bubbleDown(uint16_t idx);
        public:
            TilePriorityQueue(const Vec2<uint16_t>& grid_size, std::vector<std::string> states);
            void insert(const Cell& cell);
            bool empty();
            Cell pop();
            void print();
    };

    class WaveFunction 
    {
        private:
            Grid& _grid;
            std::unordered_map<std::string,Tile>& _tiles;
            TilePriorityQueue _queue;
            Vec2<int32_t> _shift(const Direction& direction, const Vec2<uint16_t>& vec);
            float _calculateTileWeight(const Vec2<int32_t>& pos, const Tile& tile);
        public:
            WaveFunction(Grid& grid, std::unordered_map<std::string,Tile>& tiles);
            void run();
            void collapse(const Cell& cell);
    };
}

namespace ecs 
{
    class ECS;
    extern ECS ecs;

    struct Physics
    {
        Vec2<float> vel;
        Vec2<float> acc;
        float mass;
        bool enabled {true};
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
        std::string animation_key;
        bool invertY {false};
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
            /*
            */
            template <typename T>
            T* assign(Entity* e_ptr);

            template <typename T>
            T* fetch(Entity* e_ptr);

            template <typename... T>
            bool has(Entity* e_ptr);
    };

    class EntityManager
    {
        private:
            uint16_t _max_entities;
            std::deque<eid_t> _free;
        public:
            std::vector<Entity> entities;
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
            template <typename... T>
            std::vector<Entity*> query();
    };

    class System
    {
        //TODO: improve this by adding sampling frequency
        private:
            float _sample_frequency;
        public:
            virtual void update(float dt);
    };

    class PhysicsSystem : public System
    {
        public:
            void update(float dt) override;
    };

    class PlanetPhysicsSystem : public System
    {
        public:
            void update(float dt) override;
    };

    class RenderSystem : public System
    {
        public:
            void update(float dt) override;
    };
}
#include "jlo.tpp"