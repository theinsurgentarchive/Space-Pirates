#include "jlo.h"
#include "fonts.h"
#include <cstdint>
#include <bitset>
#include <memory>
#include <vector>
#include <array>
#include <deque>
#include <iostream>

void show_jlo(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Developer - Justin Lo");
}
namespace wfc
{
    Cell::Cell(std::unordered_set<std::string> states, Vec2<uint16_t> pos)
    : states{states},pos{pos} {}

    uint16_t Cell::entropy()
    {
        return static_cast<uint16_t>(states.size());
    }

    TileMetaBuilder& TileMetaBuilder::setWeight(uint16_t w)
    {
        _weight = w;
        return *this;
    }

    TileMetaBuilder& TileMetaBuilder::addRule(Direction d, std::string t_name)
    {
        _rules[d].push_back(t_name);
        return *this;
    }

    TileMeta TileMetaBuilder::build()
    {
        TileMeta meta;
        meta.weight = _weight;
        meta.rules = _rules;
        return meta;
    }

    void TileMetaContainer::insert(const std::string& t_name, const TileMeta& meta)
    {
        _tile_map.insert({t_name,meta});
    }

    std::vector<std::string> TileMetaContainer::keys()
    {
        std::vector<std::string> v;
        v.reserve(_tile_map.size());
        for (const auto& p: _tile_map)
            v.emplace_back(p.first);
        return v;
    }

    std::vector<TileMeta> TileMetaContainer::values()
    {
        std::vector<TileMeta> v;
        v.reserve(_tile_map.size());
        for (const auto& p : _tile_map)
            v.emplace_back(p.second);
        return v;
    }

    TileMeta& TileMetaContainer::operator[](const std::string& t_name)
    {
        auto it = _tile_map.find(t_name);
        if (it == _tile_map.end()) {
            throw std::out_of_range("tile not found");
        }
        return it->second;
    }

    void TilePriorityQueue::_swap(int idx_one, int idx_two)
    {
        auto temp = _queue[idx_one];
        _queue[idx_one] = _queue[idx_two];
        _queue[idx_two] = temp;
    }

    void TilePriorityQueue::_bubble_up(int idx)
    {
        while (idx > 0) {
            uint16_t parent = (idx - 1) / 2;
            if (_queue[parent].entropy() > _queue[idx].entropy())
                _swap(parent,idx);
            else
                break;
        }
    }

    void TilePriorityQueue::_bubble_down([[maybe_unused]]int idx)
    {
        //size_t size = _queue.size();
    }

    Grid::Grid(uint16_t width, uint16_t height)
    : _width{width},_height{height} 
    {
        _grid.resize(height);
        for (auto& row : _grid)
            row.resize(width);
    }

    void Grid::set(std::string t_name, Vec2<uint16_t> pos)
    {
        _grid[pos[0]][pos[1]] = t_name;
    }

    std::string Grid::get(Vec2<uint16_t> pos)
    {
        return _grid[pos[0]][pos[1]];
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
            _entities.push_back({i, cmask_t()});
            _free.push_back(i);
        }
    }

    Entity *EntityManager::get(eid_t idx)
    {
        return &_entities[idx];
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

    System::~System()=default;

    void System::update([[maybe_unused]]float dt) {}

}