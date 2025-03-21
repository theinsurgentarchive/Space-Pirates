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

#include "image.h"
#include "jlo.h"
#include "fonts.h"
#include "image.h"

void show_jlo(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Developer - Justin Lo");
}

std::vector<std::string> img_extensions {"png","jpeg","jpg","gif"};
std::random_device rd;
std::mt19937 generator(rd());
Vec2<int32_t> dirs[4] {{0,1},{0,-1},{-1,0},{1,0}};
Direction opposite[4] {BOTTOM,TOP,RIGHT,LEFT};

extern std::unordered_map<
    std::string,
    std::shared_ptr<Texture>> textures;
extern std::unordered_map<
    std::string,
    std::shared_ptr<Animation>> animations;
extern std::unique_ptr<unsigned char[]> buildAlphaData(Image *img);

Texture::Texture(
        const Vec2<uint16_t>& dim, 
        bool alpha) : 
        dim{dim}, 
        alpha{alpha}
{
    tex = std::make_shared<GLuint>();
}

std::shared_ptr<Texture> TextureLoader::load(
        const std::string& file_name, 
        bool alpha)
{
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
            Vec2<uint16_t>{img.width,img.height},
            alpha);
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

// void TextureLoader::loadFolder(
// const std::string& folder_name, 
// bool alpha)
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
    const std::array<Vec2<uint16_t>,2>& frame_range) : 
    _texture_key {texture_key},
    _sprite_dim {sprite_dim},
    _frame_dim {frame_dim},
    _frame_range {frame_range} 
{
}

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

AnimationBuilder& AnimationBuilder::setTextureKey(
        const std::string& texture_key)
{
    _texture_key = texture_key;
    return *this;
}

AnimationBuilder& AnimationBuilder::setSpriteDimension(
        const Vec2<uint16_t>& sprite_dim)
{
    _sprite_dim = sprite_dim;
    return *this;
}

AnimationBuilder& AnimationBuilder::setFrameDimension(
        const Vec2<uint16_t>& frame_dim)
{
    _frame_dim = frame_dim;
    return *this;
}
AnimationBuilder& AnimationBuilder::setFrameRange(
        const std::array<Vec2<uint16_t>,2>& frame_range)
{
    _frame_range = frame_range;
    return *this;
}


std::shared_ptr<Animation> AnimationBuilder::build()
{
    return std::make_shared<Animation>(
            _texture_key,
            _sprite_dim,
            _frame_dim,
            _frame_range);
}

namespace wfc
{
    Tile::Tile(
        float weight, 
        std::array<std::unordered_set<std::string>,4>& rules, 
        std::unordered_map<std::string,float>& coefficients) 
        : 
        weight{weight}, 
        rules{rules}, 
        coefficients{coefficients} 
    {        
    }

    TileBuilder& TileBuilder::weight(float weight) 
    {
        _weight = weight;
        return *this;
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

    TileBuilder& TileBuilder::coefficient(const std::string& tile, float weight)
    {
        _coefficients[tile] = weight;
        return *this;
    }

    Tile TileBuilder::build()
    {
        return {_weight,_rules,_coefficients};
    }

    Cell::Cell(
        const Vec2<int32_t>& pos, 
        const std::unordered_set<std::string>& states) 
        : 
        pos{pos},
        states{states} 
    { 
    }

    uint16_t Cell::entropy() const
    {
        return static_cast<uint16_t>(states.size());
    }

    bool Cell::collapsed() const
    {
        return !state.empty();
    }
    
    Grid::Grid(
        Vec2<uint16_t> size, 
        const std::unordered_set<std::string>& states)
        : 
        _size{size} 
    {
        _cells.resize(size[1]);
        for (int i {0}; i < size[1]; i++) {
            auto& row = _cells[i];
            row.reserve(size[0]);
            for (int j {0}; j < size[0]; j++) {
                row.emplace_back(Vec2<int32_t>(j,i),states);
            }
        }
    }

    Vec2<uint16_t> Grid::size() const
    {
        return _size;
    }

    Cell* Grid::get(Vec2<int32_t> pos)
    {
        if (pos[0] >= 0 && 
            pos[0] < _size[0] && 
            pos[1] >= 0 && 
            pos[1] < _size[1]) {
            return &_cells[pos[0]][pos[1]];
        }
        return nullptr;
    }

    std::vector<std::vector<Cell>>& Grid::getCells()
    {
        return _cells;
    }

    void Grid::print()
    {
        for (auto& rows : _cells) {
            for (auto& str : rows) {
                if (str.state == "A") {
                    std::cout << "\033[92m" << str.state << "\033[0m ";
                } else if (str.state == "_") {
                    std::cout << "\033[94m" << str.state << "\033[0m ";
                } else {
                    std::cout << "\033[93m" << str.state << "\033[0m ";
                }
                //std::cout << str.entropy() << ' ';
            }
            std::cout << '\n';
        }
    }

    bool Grid::collapsed()
    {
        for (auto& row : _cells) {
            for (auto& cell : row) {
                if (!cell.collapsed()) {
                    return false;
                }
            }
        }
        return true;
    }

    TilePriorityQueue::TilePriorityQueue(Grid& grid)
    {
        for (auto& rows : grid.getCells()) {
            for (auto& cell : rows) {
                _queue.push_back(&cell);
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
            if (_queue[parent]->entropy() > _queue[idx]->entropy()) {
                _swap(parent,idx);
            } else {
                break;
            }
        }
    }

    void TilePriorityQueue::_bubbleDown(uint16_t idx)
    {
        auto size = _queue.size();
        while (idx <= size) {
            uint16_t left = 2 * idx + 1;
            uint16_t right = 2 * idx + 2;
            uint16_t smallest = idx;
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
        std::unordered_map<std::string,Tile>& tiles) 
        : 
        _grid{grid}, 
        _tiles{tiles}, 
        _queue{grid} {
        std::vector<std::string> states;
        for (auto& pair : _tiles)
            states.push_back(pair.first);
        _queue = {grid};
    }

    Vec2<int32_t> _shift_vector(uint16_t dir, const Vec2<int32_t>& vec) {
        return Vec2<int32_t> {vec[0] + dirs[dir][0],vec[1] + dirs[dir][1]};
    } 

    float WaveFunction::_calculateTileWeight(
            const Vec2<int32_t>& pos, 
            const Tile& tile)
    {
        auto base = tile.weight;
        auto weight = base;
        for (int i = 0; i < 4; i++) {
            auto shift = pos + dirs[i];
            auto cell = _grid.get(shift);
            if (shift[0] >= 0 && 
                shift[0] < _grid.size()[0] && 
                shift[1] >= 0 && 
                shift[1] < _grid.size()[1] && 
                cell->collapsed()) {
                if (tile.coefficients.find(cell->state) != 
                        tile.coefficients.end()) {
                    auto coefficient = tile.coefficients.at(cell->state);
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
                    
                    auto s1 = std::find(
                        t2_rules.begin(),
                        t2_rules.end(),
                        *it) != t2_rules.end();

                    auto s2 = std::find(
                        t1_rules.begin(),
                        t1_rules.end(),
                        c2->state) != t1_rules.end();

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

    EntityManager::EntityManager(
            uint16_t max_entities) :
            _max_entities{max_entities}
    {
        for (uint16_t i{0}; i < _max_entities; i++) {
            entities.push_back({i, cmask_t()});
            _free.push_back(i);
        }
    }

    Entity *EntityManager::checkout()
    {
        if (_free.empty())
            return nullptr;
        eid_t idx{_free.front()};
        _free.pop_front();
        return &entities[idx];
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
            }
        }
    }
    
    void RenderSystem::update([[maybe_unused]]float dt)
    {
        auto entities = ecs::ecs.query<SPRITE,TRANSFORM>();

        for (auto& entity : entities) {
            auto ec = ecs.component().fetch<SPRITE>(entity);
            if (ec->animation_key.empty()) {
                DWARNF("animation key was empty for entity (%d)\n", 
                        entity->id);
                continue;
            }
            
            auto a = animations[ec->animation_key];
            if (a == nullptr) {
                DWARNF("animation from animation key: %s was null\n", 
                        ec->animation_key.c_str());
                continue;
            }

            auto texture_key = a->getTextureKey();
            if (texture_key.empty()) {
                DWARN("texture key was empty");
                continue;
            }

            std::shared_ptr<Texture> tex = textures[texture_key];
            if (tex == nullptr) {
                DWARNF("texture from texture key: %s was null\n",
                        texture_key.c_str());
                continue;
            }
            Vec2<uint16_t> sprite_dim = a->getSpriteDim();
            Vec2<uint16_t> frame_dim = a->getFrameDim();
            uint16_t frame {a->getFrame()};
            uint16_t sw {sprite_dim[0]};
            uint16_t sh {sprite_dim[1]};
            uint16_t rows {frame_dim[0]};
            uint16_t columns {frame_dim[1]};
            uint16_t ix {static_cast<uint16_t>(frame / columns)};
            uint16_t iy {static_cast<uint16_t>(frame % columns)};
            float fx {(float) ix / columns};
            float fy {(float) iy / rows};
            float xo {(float) 1 / columns};
            float xy {(float) 1 / rows};
            DINFOF("rendering texture (%s) at (%i,%i)\n",
                    texture_key.c_str(),ix,iy);
            glBindTexture(GL_TEXTURE_2D,*tex->tex);
            glEnable(GL_ALPHA_TEST);
	        glAlphaFunc(GL_GREATER, 0.0f);
            glColor4ub(255,255,255,255);
            glBegin(GL_QUADS);
                auto tc = ecs.component().fetch<TRANSFORM>(entity);
                glTexCoord2f(fx, fy + xy);      
                glVertex2i(tc->pos[0] - sw, tc->pos[1] - sh);
                glTexCoord2f(fx, fy);          
                glVertex2i(tc->pos[0] - sw, tc->pos[1] + sh);
                glTexCoord2f(fx + xo, fy);      
                glVertex2i(tc->pos[0] + sw, tc->pos[1] + sh);
                glTexCoord2f(fx + xo, fy + xy); 
                glVertex2i(tc->pos[0] + sw, tc->pos[1] - sh);
            glEnd();
            glBindTexture(GL_TEXTURE_2D,0);
            glDisable(GL_ALPHA_TEST);
        }
    }
}