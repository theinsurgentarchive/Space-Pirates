#pragma once
#include "image.h"
#include "asteroids/fonts.h"
#include "asteroids/defs.h"
#include <cstdint>
#include <string>
#include <bitset>
#include <memory>
#include <queue>
#include <vector>
#include <GL/glx.h>
#include <unordered_map>

class Entity;
class ComponentPool;
class Scene;
struct Texture;
class TextureLoader;
class Animation;
extern std::unordered_map<std::string,std::shared_ptr<Texture>> textures;
void show_jlo(Rect* r);
constexpr uint16_t MAX_ENTITY_COMPONENTS {32};
typedef std::bitset<MAX_ENTITY_COMPONENTS> ComponentMask;
typedef uint32_t EntityID;
extern uint16_t counter;
template <typename T>
uint16_t getComponentId() 
{
    static uint16_t cid = counter++;
    return cid;
}

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

struct Texture 
{
    uint16_t width, height;
    std::shared_ptr<GLuint> texture;
    Texture(uint16_t w, uint16_t h);
};

class TextureLoader
{
    private:
        const char* _folder_name;
        void _find_texture_names(std::vector<std::string>& t_file_names);
    public:
        TextureLoader(const char* f);
        void load_textures(std::unordered_map<std::string,std::shared_ptr<Texture>>& textures);
        
};

class EntitySystem
{
    public:
        virtual ~EntitySystem();
        virtual void update(Scene& scene, float dt);
};

class PhysicsSystem : public EntitySystem 
{
    public:
        PhysicsSystem();
        void update(Scene& scene, float dt) override;
};

class RenderSystem: public EntitySystem 
{
    public:
        RenderSystem();
        void update(Scene& scene, float dt) override;
};

class EntitySystemManager
{
    private:
        std::vector<std::shared_ptr<EntitySystem>> systems;
    public:
        void update(Scene& scene, float dt);
        template <typename T>
        std::weak_ptr<T> registerSystem();
        template <typename T>
        bool hasSystem();
};

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
        Vec2 operator+ (Vec2 v) const;
        Vec2 operator- (Vec2 v) const;
        float operator[] (int i) const;
        float& operator[] (int i);
        Vec2& operator+= (const Vec2& v);
        Vec2& operator*= (float scale);
        Vec2 operator* (float scale) const;
        float length() const;
};

class vec2i
{
    private:
        int pos[2];
    public:
        vec2i();
        vec2i(int x, int y);
        int x() const;
        int y() const;
        void x(int x);
        void y(int y);
        vec2i operator-() const;
        int operator[] (int i) const;
        int& operator[] (int i);
        vec2i& operator+= (const vec2i& v);
};

enum Direction 
{
    NORTH,
    EAST,
    SOUTH,
    WEST,
    NORTH_EAST,
    SOUTH_EAST,
    NORTH_WEST,
    SOUTH_WEST
};

class Transform
{
    public:
        Vec2 pos, scale;
        float rotation;
        Transform();
        Direction direction();
};

class Sprite
{
    public:
        std::unordered_map<std::string,std::shared_ptr<Animation>> animations;
        uint16_t render_order;
        std::string c_anim;
};

struct Physics
{
    Vec2 velocity;
    Vec2 acceleration;
    bool physics {true};
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

struct AnimationFrame 
{
    vec2i f_idx;
};

class Animation 
{
    private:
        std::string _t_key;
        vec2i _s_dim;
    public:
        uint16_t frame;
        std::vector<AnimationFrame> frames;
        int sprite_width() const;
        int sprite_height() const;
        std::string texture_key() const;
        Animation(std::string t, vec2i s_dim);
        Animation(std::string t, vec2i s_dim, uint16_t f);
        void step();
};

class AnimationBuilder 
{
    private:
        std::string _t_key;
        vec2i _s_dim;
        uint16_t _frame;
        std::vector<AnimationFrame> _frames;
    public:
        AnimationBuilder(std::string t, vec2i s_dim, uint16_t f);
        AnimationBuilder& addFrame(int ix, int iy);
        Animation build();
};

#include "jlo.tpp"
