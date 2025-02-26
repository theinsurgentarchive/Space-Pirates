#include "jlo.h"
#include <memory>
#include <algorithm>
#include <iostream>
#include <dirent.h>
#include <cmath>

extern unsigned char* buildAlphaData(Image *img);

//written by gordon

void show_jlo(Rect* r) {
    ggprint8b(r, 16, 0x00ffffff, "Justin Lo");
}

//Start - Entity
Entity::Entity(EntityID i, ComponentMask m) : id{i},mask{m} {}

EntityID Entity::getId() const 
{
    return id;
}

ComponentMask& Entity::getMask() 
{
    return mask;
}
//End - Entity

//Start - Component Pool
ComponentPool::ComponentPool(uint16_t e) : element_size{e}, p_data{nullptr} 
{
    p_data = std::make_unique<char[]>(element_size);
}

ComponentPool::~ComponentPool() = default;

void* ComponentPool::get(uint16_t idx) 
{
    return p_data.get() + idx * element_size;
}
//End - Component Pool

//Start - Scene
Scene::Scene(uint16_t m) : max_entities{m} {
    for (auto i {0} ; i < m; i++) {
        free_entities.push_back({static_cast<EntityID>(i),ComponentMask()});
    }
}

Entity* Scene::createEntity() 
{
    if (Scene::getActiveEntities() >= max_entities) {
        throw new std::runtime_error("You are over the max entities");
    }
    Entity entity = std::move(free_entities.front());
    free_entities.erase(free_entities.begin());
    checked_out.push_back(std::move(entity));
    return &checked_out.back();
}

void Scene::destroyEntity(Entity* ptr) 
{
    auto it = std::find_if(checked_out.begin(),checked_out.end(),[ptr](const Entity& e) {return e.getId() == ptr->getId();});
    
    if (it != checked_out.end()) {
        checked_out.erase(it);
        free_entities.push_back(*it);
    }
}

uint16_t Scene::getActiveEntities() const
{
    return static_cast<uint16_t>(checked_out.size());
};

uint16_t Scene::getComponentCount(Entity* ptr) const
{   
    ComponentMask mask = ptr->getMask();
    return mask.count();
};

//Start - TextureInfo
Texture::Texture(uint16_t w, uint16_t h) : width {w}, height {h} {
    texture = std::make_shared<GLuint>();
}
//Start - TextureLoader
TextureLoader::TextureLoader(const char* f) : _folder_name {f} {}

void TextureLoader::_find_texture_names(std::vector<std::string>& t_file_names)
{
    DIR* dir = opendir(_folder_name);
	if (dir == nullptr) {
		std::cerr << "missing textures folder at: " << _folder_name << '\n';
		exit(1);
		return;
	}

    struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
			if (entry->d_type == DT_REG) {
			std::string s { entry->d_name };
			auto idx = s.find_last_of(".");
			if (idx != std::string::npos) {
				std::string f_ext = s.substr(idx + 1);
				if (f_ext == "jpg" || f_ext == "png") {
					t_file_names.push_back(s);
				}
			}
		}
	}
	closedir(dir);
}

void TextureLoader::load_textures(std::unordered_map<std::string,std::shared_ptr<Texture>>& textures)
{
    std::vector<std::string> t_file_names;
    _find_texture_names(t_file_names);

    for (auto tfn : t_file_names) {
        char buf[128];
        sprintf(buf,"%s/%s",_folder_name,tfn.c_str());
        Image img { buf };
        std::shared_ptr<Texture> pt = std::make_shared<Texture>(img.width, img.height);
        //Start - Gordon
        glGenTextures(1, pt->texture.get());
        glBindTexture(GL_TEXTURE_2D,*pt->texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

        unsigned char *data = buildAlphaData(&img);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        free (data);
        //End - Gordon
        std::clog << "loaded texture: " << tfn << '\n';
        textures.insert({tfn, std::move(pt)});
    }
}

EntitySystem::~EntitySystem() = default;

void EntitySystem::update([[maybe_unused]] Scene& scene, [[maybe_unused]] float dt) {}

PhysicsSystem::PhysicsSystem() = default;

void PhysicsSystem::update(Scene& scene, float dt) 
{
    std::vector<Entity*> entities = scene.queryEntities<Transform, Physics>();
    for (auto ptr : entities) {
        Transform* tc = scene.getComponent<Transform>(ptr);
        Physics* pc = scene.getComponent<Physics>(ptr);
        if (pc->physics) {
            pc->velocity += pc->acceleration * dt;
            tc->pos += pc->velocity * dt;
        }
    }
}

RenderSystem::RenderSystem() = default;

void RenderSystem::update(Scene& scene, float dt) 
{
    glClear(GL_COLOR_BUFFER_BIT);
    std::vector<Entity*> entities = scene.queryEntities<Transform, Sprite>();

    for (auto e : entities) {
        Transform* tc = scene.getComponent<Transform>(e);
        EntityID id = e->getId();
        Sprite* sc = scene.getComponent<Sprite>(e);

        if (sc->c_anim == "") {
            continue;
        }
        
        std::shared_ptr<Animation> a_ptr = sc->animations[sc->c_anim];
        if (a_ptr == nullptr) {
            continue;
        }
        if (a_ptr->frame++ == 8) {
            a_ptr->frame = 0;
        } 
        AnimationFrame frame = a_ptr->frames[a_ptr->frame];
        std::shared_ptr<Texture> t = textures[a_ptr->texture_key()];
        if (t == nullptr) {
            continue;
        }

        glBindTexture(GL_TEXTURE_2D, *t->texture);
        int sw = a_ptr->sprite_width();
        int sh = a_ptr->sprite_height();
        int ix = frame.f_idx[0];
        int iy = frame.f_idx[1];
        int columns = 8;
        int rows = 1;
        float fx = (float) ix / columns;
        float fy = (float) iy / rows;
        float xo = (float) 1 / columns;
        float xy = (float) 1 / rows;
        
        
        glBegin(GL_QUADS);
            if (scene.hasComponents<Physics>(e)) {
                Physics* pc = scene.getComponent<Physics>(e);
                if (pc->velocity[0] < 0) {
                    glTexCoord2f(fx, fy + xy);      glVertex2i(tc->pos[0] - sw, tc->pos[1] - sh);
                    glTexCoord2f(fx, fy);           glVertex2i(tc->pos[0] - sw, tc->pos[1] + sh);
                    glTexCoord2f(fx + xo, fy);      glVertex2i(tc->pos[0] + sw, tc->pos[1] + sh);
                    glTexCoord2f(fx + xo, fy + xy); glVertex2i(tc->pos[0] + sw, tc->pos[1] - sh);
                } else {
                    glTexCoord2f(fx + xo, fy + xy); glVertex2i(tc->pos[0] - sw, tc->pos[1] - sh);
                    glTexCoord2f(fx + xo, fy);      glVertex2i(tc->pos[0] - sw, tc->pos[1] + sh);
                    glTexCoord2f(fx, fy);           glVertex2i(tc->pos[0] + sw, tc->pos[1] + sh);
                    glTexCoord2f(fx, fy + xy);      glVertex2i(tc->pos[0] + sw, tc->pos[1] - sh);
                }
            }
            
           
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_ALPHA_TEST);
    }
}

void EntitySystemManager::update(Scene& scene, float dt) 
{
    for (auto& p_system : systems) {
        p_system->update(scene, dt); 
    }
}

//Start - vec2f
Vec2::Vec2() : pos{0,0} {}

Vec2::Vec2(float x, float y) : pos{x,y} {}

float Vec2::getX() {
    return pos[0];
}

float Vec2::getY() {
    return pos[1];
}

void Vec2::setX(float x) {
    pos[0] = x;
}

void Vec2::setY(float y) {
    pos[1] = y;
}

Vec2 Vec2::operator-() const {
    return Vec2(-pos[0],-pos[1]);
}

Vec2 Vec2::operator+ (Vec2 v) const {
    return Vec2(pos[0] + v.pos[0], pos[1] + v.pos[1]);
}
Vec2 Vec2::operator-(Vec2 v) const {
    return Vec2(pos[0] - v.pos[0], pos[1] - v.pos[1]);
}
float Vec2::operator[] (int i) const {
    return pos[i];
}

float& Vec2::operator[] (int i) {
    return pos[i];
}

Vec2& Vec2::operator+= (const Vec2& v) {
    pos[0] += v.pos[0];
    pos[1] += v.pos[1];
    return *this;
}

Vec2& Vec2::operator*= (float scale) {
    pos[0] *= scale;
    pos[1] *= scale;
    return *this;
}

Vec2 Vec2::operator* (float scale) const {
    return Vec2(pos[0] * scale, pos[1] * scale);
}

float Vec2::length() const {
    return std::sqrt(length_squared());
}
float Vec2::length_squared() const {
    return pos[0] * pos[0] + pos[1] * pos[1];
}
//End - Vector

//start - vec2i
vec2i::vec2i() : pos{0,0} {}

vec2i::vec2i(int x, int y) : pos{x,y} {}

int vec2i::x () const {
    return pos[0];
}

int vec2i::y () const {
    return pos[1];
}

void vec2i::x(int x) {
    pos[0] = x;
}

void vec2i::y(int y) {
    pos[1] = y;
}

vec2i vec2i::operator-() const {
    return vec2i { -pos[0], -pos[1] };
}

int vec2i::operator[] (int i) const {
    return pos[i];
}

int& vec2i:: operator[] (int i) {
    return pos[i];
}

vec2i& vec2i::operator+= (const vec2i& v) {
    pos[0] += v.pos[0];
    pos[1] += v.pos[1];
    return *this;
}
//end - vec2i

//Start - Transform
Transform::Transform() : pos{0,0},scale{0,0},rotation{0}{}

Direction Transform::direction() {
    return NORTH;
}
//End - Transform
//Start - Health
Health::Health() : hp{0},maxHp{0} {}

Health::Health(float h, float m) : hp{h},maxHp{m} {}

float Health::percent() {
    return hp / maxHp;
}
//End - Health

//Start - AABBHitbox
AABBHitbox::AABBHitbox(float centerX, float centerY, float radius) {
    corners[0] = Vec2(centerX - radius, centerY - radius);
    corners[1] = Vec2(centerX + radius, centerX + radius);
}
//TODO: implement collided 

//End - AABBHitbox
Animation::Animation(std::string t, vec2i s_dim) : 
    _t_key { t } , _s_dim { s_dim }, frame { 0 } {}

Animation::Animation(std::string t, vec2i s_dim, uint16_t f) :
    _t_key { t }, _s_dim { s_dim }, frame { f } {} 

int Animation::sprite_width() const {
    return _s_dim[0];
}

int Animation::sprite_height() const {
    return _s_dim[1];
}

std::string Animation::texture_key() const {
    return _t_key;
}

void Animation::step() {
    if (frame >= frames.size()) {
        frame = 0;
    } else {
        frame++;
    }
}

AnimationBuilder::AnimationBuilder(std::string t, vec2i s_dim, uint16_t f) : 
_t_key {t}, _s_dim { s_dim }, _frame {f} {}

AnimationBuilder& AnimationBuilder::addFrame(int ix, int iy) {
    AnimationFrame f {vec2i{ix,iy}};
    _frames.push_back(f);
    return *this;
}

Animation AnimationBuilder::build() {
    Animation a { _t_key, _s_dim };
    a.frames = _frames;
    return a;
}

//End - Scene


