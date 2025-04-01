#include <cstdint>
#include <bitset>
#include <array>
#include <memory>
#include <vector>
#include <deque>
#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <climits>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <queue>

#include <GL/glx.h>
#include <dirent.h>

#include "image.h"
#include "jlo.h"
#include "fonts.h"
#include "image.h"

std::array<Biome, 13> biomes = {
    Biome(CHRISTMAS, v2f{-20.0f, 0.0f}, v2f{0.3f, 0.7f}, 
        "Cold, snowy terrain, often covered in Christmas trees."),
    Biome(RAINFOREST, v2f{25.0f, 35.0f}, v2f{0.6f, 1.0f}, 
        "Tropical, dense vegetation with high biodiversity."),
    Biome(SAVANNA, v2f{25.0f, 35.0f}, v2f{0.2f, 0.6f}, 
        "Warm grasslands with scattered trees, wet and dry seasons."),
    Biome(FOREST, v2f{10.0f, 25.0f}, v2f{0.4f, 0.8f}, 
        "Moderate temperature, lush tree-filled areas."),
    Biome(GRASSLAND, v2f{10.0f, 25.0f}, v2f{0.1f, 0.5f}, 
        "Wide open plains with grasses and few trees."),
    Biome(WOODLAND, v2f{5.0f, 20.0f}, v2f{0.2f, 0.6f}, 
        "Drier areas with scattered trees and underbrush."),
    Biome(DESERT, v2f{30.0f, 50.0f}, v2f{0.0f, 0.2f}, 
        "Hot and dry, minimal precipitation and vegetation."),
    Biome(TAIGA, v2f{-10.0f, 10.0f}, v2f{0.2f, 0.6f}, 
        "Cold, coniferous forest with long winters."),
    Biome(TUNDRA, v2f{-20.0f, -5.0f}, v2f{0.3f, 0.7f}, 
        "Frozen, barren land with little vegetation."),
    Biome(ARCTIC_DESERT, v2f{_MIN_TEMPERATURE, -10.0f}, v2f{0.0f, 0.2f}, 
        "Extremely cold and dry, often covered with ice."),
    Biome(MEADOW, v2f{-5.0f, 15.0f}, v2f{0.4f, 0.8f}, 
        "Cold, wet grasslands often found in mountainous regions."),
    Biome(WETLANDS, v2f{5.0f, 20.0f}, v2f{0.6f, 1.0f}, 
        "Wet, marshy areas with abundant aquatic life."),
    Biome(HELL, v2f{40.0f, _MAX_TEMPERATURE}, v2f{0.0f, 0.1f}, 
        "Extreme temperatures and dry conditions, inhospitable to life.")
};

void show_jlo(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Developer - Justin Lo");
}

// ----- Globals ----- 
std::random_device rd;
std::mt19937 generator(rd());
v2i dirs[4] {{0,1},{0,-1},{-1,0},{1,0}};
Direction opposite[4] {BOTTOM,TOP,RIGHT,LEFT};
extern std::unordered_map<
    std::string,
    std::shared_ptr<SpriteSheet>> ssheets;
extern std::unordered_map<
    std::string,
    std::shared_ptr<Texture>> textures;
extern std::unique_ptr<unsigned char[]> buildAlphaData(Image *img);
extern const Camera* c;
extern std::vector<ecs::Collision> cols;
extern std::atomic<bool> done;
extern ThreadPool* pool;

void loadTextures(
    std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets)
{
    SpriteSheetLoader loader {ssheets};
    loader
    .loadStatic("cherry-001", 
        loadTexture(
            "./resources/textures/decorations/cherry-001.webp", true))
    .loadStatic("cherry-002", 
        loadTexture(
            "./resources/textures/decorations/cherry-002.webp", true))
    .loadStatic("chestnut-001", 
        loadTexture(
            "./resources/textures/decorations/chestnut-001.webp", true))
    .loadStatic("chestnut-002", 
        loadTexture(
            "./resources/textures/decorations/chestnut-002.webp", true))
    .loadStatic("chestnut-003", 
        loadTexture(
            "./resources/textures/decorations/chestnut-003.webp", true))
    .loadStatic("pine-001", 
        loadTexture(
            "./resources/textures/decorations/pine-001.webp", true))
    .loadStatic("pine-002", 
        loadTexture(
            "./resources/textures/decorations/pine-002.webp", true))
    .loadStatic("pine-003", 
        loadTexture(
            "./resources/textures/decorations/pine-003.webp", true))
    .loadStatic("pine-004", 
        loadTexture(
            "./resources/textures/decorations/pine-004.webp", true))
    .loadStatic("pine-005", 
        loadTexture(
            "./resources/textures/decorations/pine-005.webp", true))
    .loadStatic("pine-001snow", 
        loadTexture(
            "./resources/textures/decorations/pine-001snow.webp", true))
    .loadStatic("pine-002snow", 
        loadTexture(
            "./resources/textures/decorations/pine-002snow.webp", true))
    .loadStatic("pine-003snow", 
        loadTexture(
            "./resources/textures/decorations/pine-003snow.webp", true))
    .loadStatic("pine-004snow", 
        loadTexture(
            "./resources/textures/decorations/pine-004snow.webp", true))
    .loadStatic("pine-005snow", 
        loadTexture(
            "./resources/textures/decorations/pine-005snow.webp", true))
    .loadStatic("player-idle",
        loadTexture(
            "./resources/textures/player/idle.webp",true),
        {1,1},{18,32})
    .loadStatic("player-front",
        loadTexture(
            "./resources/textures/player/front.webp",true),
        {1,3},{26,32},true)
    .loadStatic("player-back",
        loadTexture(
            "./resources/textures/player/back.webp",true),
        {1,3},{26,32},true)
    .loadStatic("player-right",
        loadTexture(
            "./resources/textures/player/right.webp",true),
        {1,3},{26,32},true)
    .loadStatic("sand-001",
        loadTexture(
            "./resources/textures/tiles/sand-001.webp",false))
    .loadStatic("sand-002",
        loadTexture(
            "./resources/textures/tiles/sand-002.webp",false))
    .loadStatic("sand-003",
        loadTexture(
            "./resources/textures/tiles/sand-003.webp",false))
    .loadStatic("grass-001",
        loadTexture(
            "./resources/textures/tiles/grass-001.webp",false))
    .loadStatic("grass-002",
        loadTexture(
            "./resources/textures/tiles/grass-002.webp",false))
    .loadStatic("dirt-001",
        loadTexture(
            "./resources/textures/tiles/dirt-001.webp",false))
    .loadStatic("dirt-002",
        loadTexture(
            "./resources/textures/tiles/dirt-002.webp",false))
    .loadStatic("warm-water",
        loadTexture(
            "./resources/textures/tiles/warm-water.webp",false),
        {1,3},{16,16},true)
    .loadStatic("cool-water",
        loadTexture(
            "./resources/textures/tiles/cool-water.webp",false),
        {1,3},{16,16},true)
    .loadStatic("cold-water",
        loadTexture(
            "./resources/textures/tiles/cold-water.webp",false),
        {1,3},{16,16},true);
}

std::shared_ptr<Texture> loadTexture(
    const std::string& file_name, 
    bool alpha)
{
    static std::vector<std::string> img_extensions 
    {".webp",".png",".jpeg",".jpg",".gif"};
    std::ifstream file (file_name);
    if (!file.is_open()) {
        DERRORF("file does not exist: %s\n",file_name.c_str());
        return nullptr;
    }

    std::string ext;
    for (auto& extension : img_extensions) {
        auto length = file_name.length() - extension.length();
        if (extension == file_name.substr(length)) {
            ext = extension;
            break;
        }
    } 
    if (ext.empty()) {
        DWARNF("file does not have a valid extension: %s",
                file_name.c_str());
        return nullptr;
    }
    Image img {file_name.c_str()};
    auto tex = std::make_shared<Texture>(
            v2u{img.width,img.height},
            alpha);
    glGenTextures(1,tex->tex.get());
    glBindTexture(GL_TEXTURE_2D,*tex->tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        alpha ? GL_RGBA : GL_RGB, 
        img.width, 
        img.height, 
        0, 
        alpha ? GL_RGBA : GL_RGB, 
        GL_UNSIGNED_BYTE, 
        alpha ? buildAlphaData(&img).get() : img.data.get());


    DINFOF("loaded texture: %s\n",file_name.c_str());
    return tex;
}

Biome selectBiome(float temperature, float humidity) {
    std::vector<int> indices(13);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), generator);

    for (int i : indices) {
        auto& biome = biomes[i];
        if (temperature >= biome.temperature[0] && 
            temperature <= biome.temperature[1] &&
            humidity >= biome.humidity[0] && 
            humidity <= biome.humidity[1]) {
            return biome;
        }
    }
    return biomes[0];
}

Biome::Biome(
    BiomeType type, 
    const v2f& temperature, 
    const v2f& humidity,
    const std::string& description) 
    : 
    type{type}, 
    temperature{temperature}, 
    humidity{humidity},
    description{description}
{
}

Camera::Camera(v2f& pos, const v2u dim) : pos{pos},dim{dim}
{
}

void Camera::move(v2f delta)
{
    pos += delta;
}

void Camera::update() const
{
    glTranslatef((dim[0] >> 1) - pos[0], (dim[1] >> 1) - pos[1],0);
}

bool Camera::visible(v2f curr) const
{
    float wh = dim[0] >> 1;
    float hh = dim[1] >> 1;
    v2f v1 {pos[0] - wh, pos[1] - hh};
    v2f v2 {pos[0] + wh, pos[1] + hh};
    return curr[0] >= v1[0] && curr[0] <= v2[0] &&
    curr[1] >= v1[1] && curr[1] <= v2[1];
}

Texture::Texture(
        const Vec2<u16>& dim, 
        bool alpha) : 
        dim{dim}, 
        alpha{alpha}
{
    tex = std::make_shared<GLuint>();
}

SpriteSheetLoader::SpriteSheetLoader(
    std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets) 
: 
ssheets_{ssheets}
{
}

SpriteSheetLoader& SpriteSheetLoader::loadStatic(
    const std::string& key, 
    const std::shared_ptr<Texture>& tex,
    const v2u& frame_dim,
    const v2u& sprite_dim,
    bool animated)
{
    ssheets_.insert({key,std::make_shared<SpriteSheet>(frame_dim,
        sprite_dim[0] == 0 && sprite_dim[1] == 0 ? tex->dim : sprite_dim,tex,
        animated)});
    return *this;
}

SpriteSheet::SpriteSheet(
    const v2u& frame_dim,
    const v2u& sprite_dim,
    const std::shared_ptr<Texture> tex,
    bool animated)
    :
    frame_dim{frame_dim},
    sprite_dim{sprite_dim},
    tex{tex},
    animated{animated}
{
}

u16 SpriteSheet::maxFrames() const
{
    return sprite_dim[0] * sprite_dim[1];
}

void SpriteSheet::render(u16 frame, v2f pos, v2f scale, bool invertY)
{
    v2u f {
        static_cast<u16>(frame % frame_dim[1]), 
        static_cast<u16>(frame / frame_dim[1])
    };
    const u16 sw {static_cast<u16>(sprite_dim[0] * scale[0])};
    const u16 sh {static_cast<u16>(sprite_dim[1] * scale[1])};
    const u16 swhalf {sw >> 1};
    const u16 shhalf {sh >> 1};
    const u16 rows {frame_dim[0]};
    const u16 cols {frame_dim[1]};
    const float fx {(float) f[0] / cols};
    const float fy {(float) f[1] / rows};
    const float xo {(float) 1 / cols};
    const float xy {(float) 1 / rows};
    glBindTexture(GL_TEXTURE_2D,*tex->tex);
    glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0f);
    glColor4ub(255,255,255,255);
    glBegin(GL_QUADS);
        if (invertY) {
            glTexCoord2f(fx + xo, fy + xy);     
            glVertex2i(pos[0] - swhalf, pos[1] - shhalf);
            glTexCoord2f(fx + xo, fy);          
            glVertex2i(pos[0] - swhalf, pos[1] + shhalf);
            glTexCoord2f(fx, fy);     
            glVertex2i(pos[0] + swhalf, pos[1] + shhalf);
            glTexCoord2f(fx, fy + xy);
            glVertex2i(pos[0] + swhalf, pos[1] - shhalf);
        } else {
            glTexCoord2f(fx, fy + xy);     
            glVertex2i(pos[0] - swhalf, pos[1] - shhalf);
            glTexCoord2f(fx, fy);          
            glVertex2i(pos[0] - swhalf, pos[1] + shhalf);
            glTexCoord2f(fx + xo, fy);     
            glVertex2i(pos[0] + swhalf, pos[1] + shhalf);
            glTexCoord2f(fx + xo, fy + xy);
            glVertex2i(pos[0] + swhalf,pos[1] - shhalf);
        }
    glEnd();
    glDisable(GL_ALPHA_TEST);
}


std::vector<std::vector<ecs::Entity*>>& World::tiles()
{
    return _grid;
}

World::~World()
{
    for (auto& row : _grid) {
        for (auto& tile : row) {
            ecs::ecs.entity().ret(tile);
        }
    }
}

World::World(
    const v2f& origin, 
    wfc::Grid& grid, 
    std::unordered_map<std::string,wfc::TileMeta>& tiles)
{
    const auto grid_size = grid.size();
    _grid.resize(grid_size[1]);
    for (i32 i {0}; i < grid_size[1]; i++) {
        _grid.resize(grid_size[0]);
        for (i32 j {0}; j < grid_size[0]; j++) {
            auto cell = grid.get({i,j});
            if (cell == nullptr || cell->state.empty())
                continue;
            auto tile = tiles.find(cell->state);
            if (tile == tiles.end())
                continue;

            auto ssheet = ssheets.find(tile->second.ssheet);
            if (ssheet == ssheets.end())
                continue;
            auto sd = ssheet->second->sprite_dim;
            auto p = cell->pos;
            auto e = ecs::ecs.entity().checkout();
            if (e == nullptr) {
                continue;
            }
            auto tc = ecs::ecs.component().assign<TRANSFORM>(e);
            tc->scale = {3,3};
            tc->pos = origin + v2f {
                static_cast<float>(sd[0] * p[0] * tc->scale[0]),
                static_cast<float>(sd[1] * p[1] * tc->scale[1])
            };
            auto sc = ecs::ecs.component().assign<SPRITE>(e);
            sc->ssheet = tile->second.ssheet;
            
            if (sc->ssheet == "warm-water") {
                auto collider = ecs::ecs.component().assign<COLLIDER>(e);
                collider->dim = v2u {16 * 3,16 * 3};
            }
            _grid[i].push_back(e);
        }
    }
}

namespace wfc
{
    TileMeta::TileMeta(
        float weight, 
        std::string ssheet,
        std::array<std::unordered_set<std::string>,4>& rules, 
        std::unordered_map<std::string,float>& coefficients) 
        : 
        weight{weight}, 
        ssheet{ssheet},
        rules{rules}, 
        coefficients{coefficients} 
    {        
    }

    TileBuilder::TileBuilder(
        float weight, 
        const std::string& ssheet)
        :
        _weight{weight},
        _ssheet{ssheet}
    {
    }

    TileBuilder& TileBuilder::rule(int dir, const std::string& tile)
    {
        _rules[dir].insert(tile);
        return *this;
    }

    TileBuilder& TileBuilder::omni(const std::string& tile)
    {
        for (int i = 0; i < 4; i++)
            _rules[i].insert(tile);
        return *this;
    }

    TileBuilder& TileBuilder::coefficient(
            const std::string& tile, 
            float weight)
    {
        _coefficients[tile] = weight;
        return *this;
    }

    TileMeta TileBuilder::build()
    {
        return {_weight,_ssheet,_rules,_coefficients};
    }

    Cell::Cell(
        const v2i& pos, 
        const std::unordered_set<std::string>& states) 
        : 
        pos{pos},
        states{states} 
    { 
    }

    u16 Cell::entropy() const
    {
        return static_cast<u16>(states.size());
    }

    bool Cell::collapsed() const
    {
        return !state.empty();
    }

    Grid::Grid(
        Vec2<u16> size, 
        const std::unordered_set<std::string>& states)
        : 
        size_{size} 
    {
        cells_.reserve(size[0] * size[1]);
        for (int i {0}; i < size[0]; i++) {
            for (int j {0}; j < size[1]; j++) {
                cells_.emplace_back(v2i(i,j),states);
            }
        }
    }

    Vec2<u16> Grid::size() const
    {
        return size_;
    }

    Cell* Grid::get(v2i pos)
    {
        if (pos[0] >= 0 && 
            pos[0] < size_[0] && 
            pos[1] >= 0 && 
            pos[1] < size_[1]) {
            return &cells_[pos[0] * size_[0] + pos[1]];
        }
        return nullptr;
    }

    std::vector<Cell>& Grid::cells()
    {
        return cells_;
    }

    bool Grid::collapsed()
    {
        for (auto& cell : cells_) {
            if(!cell.collapsed()) {
                return false;
            }
        }
        return true;
    }

    TilePriorityQueue::TilePriorityQueue(Grid& grid)
    {
        for (auto& cell : grid.cells()) {
            _queue.push_back(&cell);
        }
        std::shuffle(_queue.begin(),_queue.end(),generator);
    }

    void TilePriorityQueue::_swap(u16 i1, u16 i2)
    {
        auto temp = _queue[i1];
        _queue[i1] = _queue[i2];
        _queue[i2] = temp;
    }

    void TilePriorityQueue::_bubbleUp(u16 idx)
    {
        while (idx > 0) {
            u16 parent {static_cast<u16>((idx - 1) / 2)};
            if (_queue[parent]->entropy() > _queue[idx]->entropy()) {
                _swap(parent,idx);
            } else {
                break;
            }
        }
    }

    void TilePriorityQueue::_bubbleDown(u16 idx)
    {
        auto size = _queue.size();
        while (idx <= size) {
            u16 left = 2 * idx + 1;
            u16 right = 2 * idx + 2;
            u16 smallest = idx;
            if (left < size && 
                _queue[left]->entropy() < _queue[smallest]->entropy()) {
                smallest = left;
            }
            if (right < size && 
                _queue[right]->entropy() < _queue[smallest]->entropy()) {
                smallest = right;
            }
            if (smallest == idx) {
                break;
            }
            _swap(smallest,idx);
            idx = smallest;
        }
    }

    void TilePriorityQueue::insert(Cell* cell)
    {
        _queue.push_back(cell);
        _bubbleUp(_queue.size() - 1);
    }

    bool TilePriorityQueue::empty()
    {
        return _queue.empty();
    }

    Cell* TilePriorityQueue::pop()
    {
        auto cell = _queue.front();
        _queue[0] = _queue.back();
        _queue.pop_back();
        _bubbleDown(0);
        return cell;
    }

    WaveFunction::WaveFunction(
        Grid& grid, 
        std::unordered_map<std::string,TileMeta>& tiles) 
        : 
        _grid{grid}, 
        _tiles{tiles}, 
        _queue{grid} {
        std::vector<std::string> states;
        for (auto& pair : _tiles)
            states.push_back(pair.first);
        _queue = {grid};
    }

    v2i _shift_vector(u16 dir, const v2i& vec) {
        return v2i {vec[0] + dirs[dir][0],vec[1] + dirs[dir][1]};
    } 

    float WaveFunction::_calculateTileWeight(
            const v2i& pos, 
            const TileMeta& meta)
    {
        auto base = meta.weight;
        auto weight = base;
        for (int i = 0; i < 4; i++) {
            auto shift = pos + dirs[i];
            auto cell = _grid.get(shift);
            if (shift[0] >= 0 && 
                shift[0] < _grid.size()[0] && 
                shift[1] >= 0 && 
                shift[1] < _grid.size()[1] && 
                cell->collapsed()) {
                if (meta.coefficients.find(cell->state) != 
                        meta.coefficients.end()) {
                    auto coefficient = meta.coefficients.at(cell->state);
                    weight += base * coefficient;
                }
            }
        }
        return weight;
    } 
    
    void WaveFunction::_collapse(Cell* c)
    {
        float cumulative_weight {0};
        std::vector<std::pair<std::string,float>> state_weights;
        for (auto& state : c->states) {

            auto it = _tiles.find(state);
            if (it != _tiles.end()) {
                auto weight = _calculateTileWeight(c->pos, it->second);
                cumulative_weight += weight;
                state_weights.push_back({state,cumulative_weight});
            }
        }

        std::uniform_real_distribution<float> dist {0.0f,cumulative_weight};

        float r = dist(generator);

        std::string selected;
        for (auto& pair : state_weights) {
            if (r <= pair.second) {
                selected = pair.first;
                break;
            }
        }
        _grid.get(c->pos)->state = selected;
    }

    void WaveFunction::_propagate(Cell* c)
    {
        for (int i {0}; i < 4; i++) {
            auto c1 = _grid.get(_shift_vector(i,c->pos));
            if (c1 == nullptr || c1->collapsed())
                continue;
            for (int j {0}; j < 4; j++) {
                auto c2 = _grid.get(_shift_vector(j,c1->pos));
                if (c2 == nullptr || !c2->collapsed())
                    continue;
                auto t2 = _tiles.find(c2->state);
                if (t2 == _tiles.end())
                    continue;
                auto t2_rules = t2->second.rules[opposite[j]];
                for (auto it = c1->states.begin(); it != c1->states.end();) {
                    auto t1 = _tiles.find(*it);
                    auto t1_rules = t1->second.rules[j];
                    
                    auto s1 = std::count(
                        t2_rules.begin(),
                        t2_rules.end(),
                        *it);

                    auto s2 = std::count(
                        t1_rules.begin(),
                        t1_rules.end(),
                        c2->state);

                    bool compatible = s1 && s2;

                    if (c1->entropy() == 1)
                        break;

                    if (compatible) {
                        ++it;
                    } else {
                        it = c1->states.erase(it);
                    }
                }
            }
        }
    }

    void WaveFunction::run()
    {
        while (!_queue.empty()) {
            if (_grid.collapsed())
                break;
            auto cell = _queue.pop();
            if (cell == nullptr)
                break;
            _collapse(cell);
            _propagate(cell);
        }
    }
}

namespace ecs
{   
    ECS ecs;

    Entity::Entity(u32 i, cmask m) : id(i), mask(m) {}

    ComponentPool::ComponentPool(u32 size) : _size {size}
    {
        _ptr_data = std::make_unique<char[]>(size * MAX_ENTITIES);
    }

    u32 ComponentPool::size() const
    {
        return _size;
    }

    void *ComponentPool::get(u16 idx)
    {
        return _ptr_data.get() + idx * _size;
    }

    EntityManager::EntityManager(
            u32 max_entities) :
            _max_entities{max_entities}
    {
        for (u32 i{0}; i < _max_entities; i++) {
            entities.push_back({i, cmask()});
            _free.push_back(i);
        }
    }

    Entity *EntityManager::checkout()
    {
        if (_free.empty())
            return nullptr;
        u32 idx{_free.front()};
        _free.pop_front();
        return &entities[idx];
    }

    void EntityManager::ret(Entity* e_ptr)
    {
        e_ptr->mask.reset();
        _free.push_back(e_ptr->id);
        e_ptr = nullptr;
    }

    u32 EntityManager::maxEntities() const
    {
        return _max_entities;
    }

    ECS::ECS() : _entity_manager{MAX_ENTITIES} {}

    EntityManager &ECS::entity()
    {
        return _entity_manager;
    }
    
    bool EntityManager::isFree(const Entity* e_ptr)
    {
        return std::count(_free.begin(),_free.end(),e_ptr->id);
    }

    ComponentManager &ECS::component()
    {
        return _component_manager;
    }

    std::unordered_map<u32,Transform*> transforms;
    std::unordered_map<u32,Sprite*> sprites;
    std::unordered_map<u32,Physics*> physics;
    std::unordered_map<u32,Collider*> colliders;
    
    template <typename T>
    T* getCachedComponent(const Entity* e_ptr, std::unordered_map<u32,T*>& components)
    {
        const u32 id = e_ptr->id;
        static std::mutex component_mutex;

        std::lock_guard<std::mutex> lock(component_mutex);
        if (ecs::ecs.entity().isFree(e_ptr)) {
            return nullptr;
        }
        if (components.count(id) == 0) {
            auto component = ecs::ecs.component().fetch<T>(e_ptr);
            components[id] = component;
            return component;
        }

        return components[id];
    }


    PhysicsSystem::PhysicsSystem(ECS& ecs, float sample_delta) 
    :
    System<Transform,Physics>(ecs,sample_delta)
    {
    }

    void PhysicsSystem::update(float dt)
    {
        for (auto& entity : _entities) {
            DINFOF("applying physics to entity (%d)\n",entity->id);
            auto tc = _ecs.component().fetch<Transform>(entity);
            auto pc = _ecs.component().fetch<Physics>(entity);
            if (pc->enabled) {
                pc->vel += pc->acc * dt;
                tc->pos += pc->vel * dt;
            }
        }
    }

    RenderSystem::RenderSystem(ECS& ecs, float sample_delta) 
    :
    System<Transform,Sprite>(ecs,sample_delta)
    {
    }

    void RenderSystem::sample()
    {
        _entities = _ecs.query<SPRITE,TRANSFORM>();
        
    }
    
    struct CompareEntity
    {
        bool operator()(const Entity* e1, const Entity* e2)
        {
            auto s1 = getCachedComponent<Sprite>(e1, sprites);
            auto s2 = getCachedComponent<Sprite>(e2, sprites);
            return s1->render_order > s2->render_order;
        }
    };

    void renderHelper(std::vector<const Entity*>& entities, 
        std::priority_queue<const Entity*, std::vector<const Entity*>, CompareEntity>& visible, 
        std::mutex& mutex, u32 start, u32 end)
    {
        std::vector<const Entity*> local_visible;
        local_visible.reserve(end - start);
        for (int i {start}; i < end; ++i) {
            auto e = entities[i];
            Transform* tc = ecs::ecs.component().fetch<Transform>(e);
            if ((c->visible(tc->pos))) {
                local_visible.emplace_back(e);
            }
        }

        std::lock_guard<std::mutex> lock(mutex);
        for (auto& entity : local_visible) {
            visible.push(entity);
        }
    }
  
    void RenderSystem::update([[maybe_unused]]float dt)
    {
        auto entities = ecs::ecs.query<Sprite,Transform>();
        std::mutex mx;
        const u32 nthreads = 4;
        const u32 e_threads = entities.size() / nthreads;
        std::priority_queue<const Entity*, std::vector<const Entity*>, CompareEntity> visible;
        std::vector<std::thread> threads;
        for (u32 i = 0; i < nthreads; ++i) {
            u32 start = i * e_threads;
            u32 end = (i == nthreads - 1) ? entities.size() : start + e_threads;
            threads.push_back(std::thread(renderHelper,std::ref(entities),std::ref(visible), std::ref(mx), start,end));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        while (!visible.empty()) {
            auto entity = visible.top();
            visible.pop();
            auto tc = getCachedComponent<Transform>(entity,transforms);
            if (!c->visible(tc->pos)) {
                DINFO("entity was skipped");
                continue;
            }
            auto ec = getCachedComponent<Sprite>(entity,sprites);
            if (ec->ssheet.empty()) {
                DWARNF("animation key was empty for entity (%d)\n", 
                        entity->id);
                continue;
            }
            
            auto ssheet = ssheets[ec->ssheet];
            if (ssheet == nullptr) {
                DWARNF("animation from animation key: %s was null\n", 
                        ec->ssheet.c_str());
                continue;
            }
            
            if (ssheet->tex == nullptr) {
                continue;
            }
            ssheet->render(ec->frame,tc->pos, tc->scale, ec->invert_y);
            if (ssheet->animated) {
                ec->frame++;
                auto f = ssheet->frame_dim;
                if (ec->frame >= f[0] * f[1]) {
                    ec->frame = 0;
                }
            }

            if (_ecs.component().has<NAME>(entity)) {
                auto nc = _ecs.component().fetch<NAME>(entity);
                Rect r;
                u32 ot = (nc->name.length() >> 1) * 2;
                switch(nc->alignment) {
                    case 0:
                        r.left = tc->pos[0] + ot;
                        r.bot = tc->pos[1];
                        break;
                    case 1:
                    case 2:
                        break;
                }
                r.left += nc->offset[0];
                r.bot += nc->offset[1];
                ggprint8b(&r,0,nc->cref,nc->name.c_str());
            }
            if (_ecs.component().has<COLLIDER>(entity)) {

            }
        }
    }

    bool checkCollision(
        const Transform* ta, 
        const Transform* tb, 
        const Collider* ca, 
        const Collider* cb)
    {
        const v2f a = ta->pos + ca->offset;  // Box A position
        const v2f b = tb->pos + cb->offset;  // Box B position

        return 
            (a[0] - ca->dim[0] * 0.5f < b[0] + cb->dim[0] * 0.5f) &&
            (a[0] + ca->dim[0] * 0.5f > b[0] - cb->dim[0] * 0.5f) &&
            (a[1] + ca->dim[1] * 0.5f > b[1] - cb->dim[1] * 0.5f) &&
            (a[1] - ca->dim[1] * 0.5f < b[1] + cb->dim[1] * 0.5f);

    }
    
    
    

    CollisionSystem::CollisionSystem(ECS& ecs, float sample_delta) 
    :
    System<Transform,Collider>(ecs,sample_delta)
    {

    }

    // void collisionHelper(std::vector<const Entity*>& entities, 
    //     std::vector<const Entity*>& visible, 
    //     std::mutex& mutex, u32 start, u32 end)
    // {
    //     std::vector<const Entity*> local_visible;
    //     local_visible.reserve(end - start);
    //     for (int i {start}; i < end; ++i) {
    //         Transform* tc = ecs::ecs.component().fetch<Transform>(entities[i]);
    //         if ((c->visible(tc->pos))) {
    //             local_visible.emplace_back(entities[i]);
    //         }
    //     }

    //     std::lock_guard<std::mutex> lock(mutex);
    //     for (auto& entity : local_visible) {
    //         visible.push_back(entity);
    //     }
    // }
    void CollisionSystem::update([[maybe_unused]] float dt)
    {   
    //     const auto entities = ecs::ecs.query<Collider, Transform>(); //query should be thread-safe?
    //     std::mutex mx;
    //     const u32 e_threads = entities.size() / 4;
    //     AtomicVector<const Entity*> visible_entities;
    //     for (u32 i = 0; i < 4; ++i) {
    //         u32 start = i * e_threads;
    //         u32 end = (i == 4 - 1) ? entities.size() : start + e_threads;
    //         if (start >= entities.size()) {
    //             std::cerr << "Invalid start index: " << start << std::endl;
    //             continue;  // Skip this iteration if the start is invalid
    //         }
    //         std::vector<const Entity*> local_entities(entities.begin() + start, entities.begin() + end);
    //         pool.enqueue([&local_entities, &visible_entities]() {
    //             collisionHelper(local_entities, visible_entities, 0, local_entities.size());
    //         });
    //     }


    //     std::unordered_map<u32,Transform*> transforms;
    //     std::unordered_map<u32,Collider*> colliders;
    //     std::unordered_set<u64> checked;
    //     for (auto& a : visible_entities) {
    //         for (auto& b : visible_entities) {
    //             if (done) {
    //                 return;
    //             }

    //             if (a->id == b->id) {
    //                 continue;
    //             }

    //             u64 hash = a->id * 54 + b->id * 23 + 33;
    //             if (checked.count(hash)) {
    //                 continue;
    //             }
    //             checked.insert(hash);

    //             Transform* ta = getCachedComponent<Transform>(a, transforms);
    //             Transform* tb = getCachedComponent<Transform>(b, transforms);

    //             if (!(c->visible(ta->pos) || c->visible(tb->pos))) {
    //                 continue;
    //             }

    //             Collider* ca = getCachedComponent<Collider>(a, colliders);
    //             Collider* cb = getCachedComponent<Collider>(b, colliders);

    //             if (checkCollision(ta, tb, ca, cb)) {
    //                 std::cout << "here" << '\n';
    //             }
    //         }
    //     }
    // }
    }
}

// void collisionHelper(
//     const std::vector<const ecs::Entity*>& entities, 
//     AtomicVector<const ecs::Entity*>& visible_entities, 
//     const u32 start, 
//     const u32 end)
// {
//     std::vector<const ecs::Entity*> thread_visible;
//     thread_visible.reserve(end - start);
//     for (u32 i {start}; i < end; i++) {
//         auto e = entities[i];
//         TRANSFORM* tc = ecs::ecs.component().fetch<TRANSFORM>(entities[i]);
//         if ((c->visible(tc->pos))) {
//             thread_visible.emplace_back(entities[i]);
//         }
//     }
//     visible_entities.add(thread_visible);
// }

ThreadPool::ThreadPool(u32 nthreads) : stop_{false}
{
    workers_.reserve(nthreads);
    for (u32 i {0}; i < nthreads; ++i) {
        workers_.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    task_available_.notify_all();

    for (auto& worker : workers_) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(task);
    }

    task_available_.notify_one();
}

void ThreadPool::workerThread()
{
    while (1) {
        std::function<void()> current_task = nullptr;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            task_available_.wait(lock, [this] { return stop_ || !task_queue_.empty(); });
            
            if (stop_ && task_queue_.empty()) {
                return;
            }

            current_task = std::move(task_queue_.front());
            task_queue_.pop();
        }

        if (current_task) {
            current_task();
        }
    }
}


    
