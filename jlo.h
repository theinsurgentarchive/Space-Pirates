#pragma once

#include <cstdint>
#include <string>
#include <bitset>
#include <memory>
#include <queue>
#include <vector>

constexpr uint16_t MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;
typedef uint32_t EntityID;

extern uint16_t counter;
template <typename T>
uint16_t getComponentId() 
{
    static uint16_t cid = counter++;
    return cid;
}

class Vec2 
{
    private:
        float pos[2];
        float length_squared() const;
    public:
        Vec2();
        Vec2(float x, float y);
        float getX();
        float getY();
        void setX(float x);
        void setY(float y);
        Vec2 operator-() const;
        float operator[] (int i) const;
        float& operator[] (int i);
        Vec2& operator+= (Vec2& v);
        Vec2& operator*= (float scale);
        float length() const;
};

class Entity 
{
    private:
        // Unique identifier for this entity
        EntityID id;
        // Bitfield can be queried to show components that are enabled for this entity
        ComponentMask mask;
    public:
        Entity(EntityID i, ComponentMask m);
        EntityID getId() const;
        ComponentMask& getMask();
        
};

/*
    An array for a singular type of component
    Credit: https://www.david-colson.com/2020/02/09/making-a-simple-ecs.html
*/
class ComponentPool
{
    private:
        uint16_t element_size;
        // Pointer to byte array for this component type
        std::unique_ptr<char[]> p_data;
    public:
        ComponentPool(uint16_t e);
        ~ComponentPool();

        // Returns a pointer to the data at 'idx'
        void* get(uint16_t idx);
};

/*
    Container to hold entity state/components
    Credit: https://austinmorlan.com/posts/entity_component_system/#the-component
*/
class Scene
{
    private:
        uint16_t max_entities;
        std::vector<Entity> free_entities;
        std::vector<Entity> checked_out;
        std::vector<std::unique_ptr<ComponentPool>> pools;
    public:
        Scene(uint16_t m);
        
        /*
            Creates an entity for this scene.
            
            @returns entity
        */
        Entity* createEntity();

        /*
            Removes the entity from this scene.
        */
        void destroyEntity(Entity* ptr);

        /*
            Adds a component 'T' to 'entity'. 
            If the component has already been added to 'entity',
            this will throw a std::runtime_error.
            
            @param entity being checked
            @returns pointer to 'T' 
        */
        template <typename T>
        T* addComponent(Entity* ptr);
        
        /*
            Returns a pointer to component 'T'.
            If the entity does not have the component, this will return a nullptr.
            This is an implementation detail.

            @params entity being checked
            @returns pointer to 'T'
        */
        template <typename T>
        T* getComponent(Entity* ptr);

        /*
            Checks whether an entity has a component type 'T'

            @param entity being checked
            @returns bool whether the entity has the component
        */
        template <typename... T>
        bool hasComponents(Entity* ptr);

        /*
            Returns the count of active entities in the scene
            
            @returns count of active entities 
        */
        uint16_t getActiveEntities() const;
        
        /* 
            Returns the count of components that belong to 'entity'
            
            @param entity being checked
            @returns count of components
        */
        uint16_t getComponentCount(Entity* ptr) const;

        /*
            Returns the list of entities that match 'T...'

            @returns a list of pointers to entities
        */
        template <typename ... T>
        std::vector<Entity*> queryEntities();

        void displayMatrix();
        
};

struct Transform
{
    Vec2 pos;
    Vec2 scale;
    float rotation;
    Transform ();
};

struct Sprite
{
    uint16_t width, height;
    std::string texture;
};

struct Velocity
{
    Vec2 velocity;
};

struct Acceleration
{
    Vec2 acceleration;
};

class Health
{
    public:
        float hp, maxHp;
        Health();
        Health(float h, float m);
        float percent();
};

class AABBHitbox
{
    public:
        Vec2 corners[2];
        AABBHitbox(float centerX, float centerY, float radius);
        bool collided(AABBHitbox hitbox);
};

#include "implementation/scene.tpp"
