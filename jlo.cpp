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

#include <GL/glx.h>
#include <dirent.h>

#include "jlo.h"
#include "fonts.h"
#include "image.h"

void show_jlo(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Developer - Justin Lo");
}

std::vector<std::string> img_extensions {"png","jpeg","jpg"};
std::random_device rd;
std::mt19937 generator(rd());
TextureLoader tl;
Vec2<int32_t> dirs[4] {{0,1},{0,-1},{-1,0},{1,0}};
Direction opposite[4] {BOTTOM,TOP,RIGHT,LEFT};

extern std::unordered_map<std::string,std::shared_ptr<Texture>> textures;
extern std::unordered_map<std::string,std::shared_ptr<Animation>> animations;
extern std::unique_ptr<unsigned char[]> buildAlphaData(Image *img);

Texture::Texture(const Vec2<uint16_t>& dim, bool alpha) 
: dim{dim}, alpha{alpha}
{
    tex = std::make_shared<GLuint>();
}

std::shared_ptr<Texture> TextureLoader::load(const std::string& file_name, bool alpha)
{
    std::ifstream file (file_name);
    if (!file.is_open()) {
        DWARNF("file does not exist: %s\n",file_name.c_str());
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
        DWARNF("file does not have a valid extension: %s",file_name.c_str());
        return nullptr;
    }
    Image img {file_name.c_str()};
    auto tex = std::make_shared<Texture>(Vec2<uint16_t>{img.width,img.height},alpha);
    glGenTextures(1,tex->tex.get());
    glBindTexture(GL_TEXTURE_2D,*tex->tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    auto data = buildAlphaData(&img);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,
        img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,data.get());
    DINFOF("loaded texture: %s\n",file_name.c_str());
    return tex;
}

// void TextureLoader::loadFolder(const std::string& folder_name, bool alpha)
// {
//     std::vector<std::string> files;

//     DIR* dir = opendir(folder_name.c_str());
//     if (dir == nullptr) {
//         DWARNF("failed to open directory: %s",folder_name.c_str());
//         return;
//     }

//     struct dirent* entry;
//     while ((entry = readdir(dir)) != nullptr) {
//         std::string file_name = entry->d_name;
//         if (file_name == "." || file_name == "..") {
//             continue;
//         }
//         std::ostringstream file_path;
//         file_path << folder_name << "/" << file_name;
//         load(file_path.str(), alpha);
//     }
//     closedir(dir);
// }

Animation::Animation(const std::string& texture_key, 
    const Vec2<uint16_t>& sprite_dim, 
    const Vec2<uint16_t>& frame_dim, 
    const std::array<Vec2<uint16_t>,2>& frame_range) 
    : 
    _texture_key {texture_key},
    _sprite_dim {sprite_dim},
    _frame_dim {frame_dim},
    _frame_range {frame_range} {}

Animation& Animation::operator+(int value)
{
    _frame += value;
    return *this;
}

Animation& Animation::operator-(int value)
{
    _frame -= value;
    return *this;
}

Animation& Animation::operator++()
{
    _frame++;
    return *this;
}

Animation& Animation::operator=(uint16_t frame)
{
    _frame = frame;
    return *this;
}

uint16_t Animation::getFrame() const
{
    return _frame;
}

uint16_t Animation::getMaxFrames() const
{
   return 0;
}

std::string Animation::getTextureKey() const
{
    return _texture_key;
}

Vec2<uint16_t> Animation::getSpriteDim() const
{
    return _sprite_dim;
}

Vec2<uint16_t> Animation::getFrameDim() const
{
    return _frame_dim;
}

AnimationBuilder& AnimationBuilder::setTextureKey(const std::string& texture_key)
{
    _texture_key = texture_key;
    return *this;
}

AnimationBuilder& AnimationBuilder::setSpriteDimension(const Vec2<uint16_t>& sprite_dim)
{
    _sprite_dim = sprite_dim;
    return *this;
}

AnimationBuilder& AnimationBuilder::setFrameDimension(const Vec2<uint16_t>& frame_dim)
{
    _frame_dim = frame_dim;
    return *this;
}
AnimationBuilder& AnimationBuilder::setFrameRange(const std::array<Vec2<uint16_t>,2>& frame_range)
{
    _frame_range = frame_range;
    return *this;
}


std::shared_ptr<Animation> AnimationBuilder::build()
{
    return std::make_shared<Animation>(_texture_key,_sprite_dim,_frame_dim,_frame_range);
}

namespace wfc
{
    Tile::Tile(float weight, 
        std::array<std::vector<std::string>,4>& rules,
        std::unordered_map<std::string,
        float>& coefficients) 
    : weight{weight}, rules{rules}, coefficients{coefficients} {}

    TileBuilder& TileBuilder::setWeight(float weight) 
    {
        _weight = weight;
        return *this;
    }

    TileBuilder& TileBuilder::addRule(const Direction& dir, 
        const std::string& tile)
    {
        _rules[dir].push_back(tile);
        return *this;
    }

    TileBuilder& TileBuilder::addCoefficient(const std::string& tile, 
        float weight)
    {
        _coefficients[tile] = weight;
        return *this;
    }

    Tile TileBuilder::build()
    {
        return {_weight,_rules,_coefficients};
    }

    Cell::Cell(std::vector<std::string> states, Vec2<int32_t> pos)
     : states{states},pos{pos} {}

    uint16_t Cell::entropy() const
    {
        return static_cast<uint16_t>(states.size());
    }
    
    Grid::Grid(Vec2<uint16_t> size) : _size{size} {
        _cells.resize(size[1]);
        for (auto& row : _cells) 
            row.resize(size[0]);
    }

    Vec2<uint16_t> Grid::size() const
    {
        return _size;
    }

    std::string Grid::get(Vec2<int32_t> pos) const
    {
        return _cells[pos[0]][pos[1]];
    }
    
    void Grid::set(Vec2<int32_t> pos, std::string name)
    {
        _cells[pos[0]][pos[1]] = name;
    }

    bool Grid::collapsed(const Vec2<int32_t>& pos)
    {
        return !_cells[pos[0]][pos[1]].empty();
    }

    void Grid::print()
    {
        for (auto& rows : _cells) {
            for (auto& str : rows) {
                if (str == "A") {
                    std::cout << "\033[92m" << str << "\033[0m ";
                } else {
                    std::cout << "\033[94m" << str << "\033[0m ";
                }
            }
            std::cout << '\n';
        }
    }

    TilePriorityQueue::TilePriorityQueue(const Vec2<uint16_t>& grid_size, std::vector<std::string> states)
    {
        uint16_t rows = grid_size[0], cols = grid_size[1];
        for (uint16_t i {0}; i < rows; i++) {
            for (uint16_t j {0}; j < cols; j++) {
                _queue.push_back({states,{i,j}});            
            }
        }
        std::shuffle(_queue.begin(),_queue.end(),generator);
    }

    void TilePriorityQueue::_swap(uint16_t i1, uint16_t i2)
    {
        auto temp = _queue[i1];
        _queue[i1] = _queue[i2];
        _queue[i2] = temp;
    }

    void TilePriorityQueue::_bubbleUp(uint16_t idx)
    {
        while (idx > 0) {
            uint16_t parent {static_cast<uint16_t>((idx - 1) / 2)};
            if (_queue[parent].entropy() > _queue[idx].entropy()) {
                _swap(parent,idx);
            } else {
                break;
            }
        }
    }
    void TilePriorityQueue::print()
    {
        for (auto p : _queue) {
            for (auto state : p.states) {
                std::cout << state << " ";
            }
            std::cout << '\n';
        }
    }

    void TilePriorityQueue::_bubbleDown(uint16_t idx)
    {
        auto size = _queue.size();
        while (idx <= size) {
            uint16_t left = 2 * idx + 1;
            uint16_t right = 2 * idx + 2;
            uint16_t smallest = idx;
            if (left < size && _queue[left].entropy() < _queue[smallest].entropy()) {
                smallest = left;
            }
            if (right < size && _queue[right].entropy() < _queue[smallest].entropy()) {
                smallest = right;
            }
            if (smallest == idx) {
                break;
            }
            _swap(smallest,idx);
            idx = smallest;
        }
    }

    void TilePriorityQueue::insert(const Cell& cell)
    {
        _queue.push_back(cell);
        _bubbleUp(_queue.size() - 1);
    }

    bool TilePriorityQueue::empty()
    {
        return _queue.empty();
    }

    Cell TilePriorityQueue::pop()
    {
        Cell cell = _queue.front();
        _queue[0] = _queue.back();
        _queue.pop_back();
        _bubbleDown(0);
        return cell;
    }

    WaveFunction::WaveFunction(Grid& grid, std::unordered_map<std::string,Tile>& tiles)
     : _grid{grid}, _tiles{tiles}, _queue{grid.size(),{}} {
        std::vector<std::string> states;
        for (auto& pair : _tiles)
            states.push_back(pair.first);
        _queue = {grid.size(),states};
    }

    Vec2<int32_t> _shift(const Direction& dir, const Vec2<uint16_t>& vec) {
        return Vec2<int32_t> {vec[0] + dirs[dir][0],vec[1] + dirs[dir][1]};
    } 

    void WaveFunction::run()
    {
        while (!_queue.empty()) {
            auto cell = _queue.pop();
            collapse(cell);
        }
    }

    float WaveFunction::_calculateTileWeight(const Vec2<int32_t>& pos, const Tile& tile)
    {
        auto base = tile.weight;
        auto weight = base;
        for (int i = 0; i < 4; i++) {
            auto shift = pos + dirs[i];
            if (shift[0] >= 0 && 
                shift[0] < _grid.size()[0] && 
                shift[1] >= 0 && 
                shift[1] < _grid.size()[1] && 
                _grid.collapsed(shift)) {
                auto t_name = _grid.get(shift);
                if (tile.coefficients.find(t_name) != tile.coefficients.end()) {
                    auto coefficient = tile.coefficients.at(t_name);
                    weight = base*coefficient + weight;
                }
            }
        }
        return weight;
    } 
    
    void WaveFunction::collapse(const Cell& cell)
    {
        float cumulative_weight {0};
        std::vector<std::pair<std::string,float>> state_weights;
        for (auto& state : cell.states) {
            auto it = _tiles.find(state);
            if (it != _tiles.end()) {
                auto weight = _calculateTileWeight(cell.pos, it->second);
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
        _grid.set(cell.pos,selected);
    }
}

namespace ecs
{   
    ECS ecs;
    
    Entity::Entity(eid_t i, cmask_t m) : id(i), mask(m) {}

    ComponentPool::ComponentPool(uint16_t size) : _size {size}
    {
        _ptr_data = std::make_unique<char[]>(_size * MAX_ENTITIES);
    }

    uint16_t ComponentPool::size() const
    {
        return _size;
    }

    void *ComponentPool::get(uint16_t idx)
    {
        return _ptr_data.get() + idx * _size;
    }

    EntityManager::EntityManager(uint16_t max_entities)
     : _max_entities{max_entities}
    {
        for (uint16_t i{0}; i < _max_entities; i++) {
            entities.push_back({i, cmask_t()});
            _free.push_back(i);
        }
    }

    Entity *EntityManager::get(eid_t idx)
    {
        return &entities[idx];
    }

    Entity *EntityManager::checkout()
    {
        if (_free.empty())
            return nullptr;
        eid_t idx{_free.front()};
        _free.pop_front();
        return get(idx);
    }

    void EntityManager::ret(Entity *&e_ptr)
    {
        e_ptr->mask.reset();
        _free.push_back(e_ptr->id);
        e_ptr = nullptr;
    }

    uint16_t EntityManager::maxEntities() const
    {
        return _max_entities;
    }

    ECS::ECS() : _entity_manager{MAX_ENTITIES} {}

    EntityManager &ECS::entity()
    {
        return _entity_manager;
    }

    ComponentManager &ECS::component()
    {
        return _component_manager;
    }

    void System::update([[maybe_unused]]float dt) {}

    void PhysicsSystem::update(float dt)
    {
        auto entities = ecs::ecs.query<TRANSFORM,PHYSICS>();

        for (auto& entity : entities) {
            DINFOF("applying physics to entity (%d)\n",entity->id);
            auto tc = ecs.component().fetch<TRANSFORM>(entity);
            auto pc = ecs.component().fetch<PHYSICS>(entity);
            if (pc->enabled) {
                pc->vel += pc->acc * dt;
                tc->pos += pc->vel * dt;
                DINFOF("final position (%f, %f) for entity (%d)\n",tc->pos[0],tc->pos[1],entity->id);
            }
        }
    }

    void RenderSystem::update(float dt)
    {
        auto entities = ecs::ecs.query<SPRITE,TRANSFORM>();

        for (auto& entity : entities) {
            auto ec = ecs.component().fetch<SPRITE>(entity);
            if (ec->animation_key.empty()) {
                DWARNF("animation key was empty for entity (%d)\n", entity->id);
                continue;
            }
            
            auto a = animations[ec->animation_key];
            if (a == nullptr) {
                DWARNF("animation from animation key: %s was null\n", ec->animation_key.c_str());
                continue;
            }

            auto texture_key = a->getTextureKey();
            if (texture_key.empty()) {
                DWARN("texture key was empty");
                continue;
            }

            std::shared_ptr<Texture> tex = textures[texture_key];
            if (tex == nullptr) {
                DWARNF("texture from texture key: %s was null\n",texture_key.c_str());
                continue;
            }
            Vec2<uint16_t> sprite_dim = a->getSpriteDim();
            Vec2<uint16_t> frame_dim = a->getFrameDim();
            uint16_t frame {a->getFrame()};
            int sw {sprite_dim[0]};
            int sh {sprite_dim[1]};
            int rows {frame_dim[0]};
            int columns {frame_dim[1]};
            uint16_t ix {frame / columns};
            uint16_t iy {frame % columns};
            float fx {(float) ix / columns};
            float fy {(float) iy / rows};
            float xo {(float) 1 / columns};
            float xy {(float) 1 / rows};
            DINFOF("rendering texture (%s) at (%i,%i)\n",texture_key.c_str(),ix,iy);
            glPushMatrix();
            glBindTexture(GL_TEXTURE_2D,*tex->tex);
            glBegin(GL_QUADS);
                auto tc = ecs.component().fetch<TRANSFORM>(entity);
                glTexCoord2f(fx, fy + xy);      glVertex2i(tc->pos[0] - sw, tc->pos[1] - sh);
                glTexCoord2f(fx, fy);           glVertex2i(tc->pos[0] - sw, tc->pos[1] + sh);
                glTexCoord2f(fx + xo, fy);      glVertex2i(tc->pos[0] + sw, tc->pos[1] + sh);
                glTexCoord2f(fx + xo, fy + xy); glVertex2i(tc->pos[0] + sw, tc->pos[1] - sh);
            glEnd();
            glBindTexture(GL_TEXTURE_2D,0);
            glPopMatrix();
        }
    }
}