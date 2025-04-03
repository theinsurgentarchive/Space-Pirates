#include "dchu.h"
#include <cmath>
#include <list>
#include <cstdlib>
#include <ctime>

namespace ecs
{
    HP_DMGSystem::HP_DMGSystem(ECS& ecs, float sample_delta)
    :
    System<Transform,Health,Combat>(ecs,sample_delta)
    {
        DINFO("Health/Damage System Initialized");
    }
    
    void HP_DMGSystem::update([[maybe_unused]]float dt)
    {
        for (auto& entity : _entities) {
            DINFOF(
                "Calculating Damage Dealt by/Afflicted to Entity (%d)\n",
                entity->id
            );
            auto tc = ecs.component().fetch<TRANSFORM>(entity);
            auto nc = ecs.component().fetch<HEALTH>(entity);
            auto dc = ecs.component().fetch<COMBAT>(entity);
            
            //To-Be-Done
        }
    }

    PathSystem::PathSystem(ECS& ecs, float sample_delta)
    :
    System<Transform,Physics,Navigate>(ecs, sample_delta)
    {
        DINFO("Path System Initialized");
    }

    void PathSystem::update([[maybe_unused]]float dt)
    {
        for (auto& entity : _entities) {
            
            DINFOF("Generating Forces for Entity (%d)\n",entity->id);
            auto tc = ecs.component().fetch<TRANSFORM>(entity);
            auto pc = ecs.component().fetch<PHYSICS>(entity);
            auto nc = ecs.component().fetch<NAVIGATE>(entity);
            
            if (pc->enabled) {
                //To-Be-Done

                //pc->vel += pc->acc * dt;
                //tc->pos += pc->vel * dt;
            }
        }
        
    }
}

//Renderability Check
bool canRender(ecs::Entity* ent)
{
    bool display = true;

    //Check entity for Transform & Sprite Component
    if (ecs::ecs.component().fetch<TRANSFORM>(ent) == nullptr) {
        display = false;
    }
    if (ecs::ecs.component().fetch<SPRITE>(ent) == nullptr) {
        display = false;
    }
    if (!display) {
        DWARNF(
            "Unrenderable Entity: ID(%d)\n", 
            tiles[x][y]->id
        );
    }
    return display;
}

float floatRand(uint16_t max, uint16_t min)
{
    //Initialize Variables
    uint16_t d_max = 99;
    uint16_t d_min = 0;
    float random = 0.0f;

    //Generate Whole Number
    uint16_t whole = (rand() % (max - min + 1) - min);
    if (whole >= max) {
        return (float)whole;
    }

    //Generate Decimal
    uint16_t decimal = (rand() % (d_max - d_min + 1) - d_min);
    
    //Generate & Return Float Number Using whole & decimal
    random += ((float)whole + (((float)decimal) / 100.0f));
    return random;
}

//A* Pathfinding Algorithm
Node::Node()
{
    //Initialize Variables
    obstacle = false;
    visited = false;
    local_dist = 0;
    global_dist = 0;
    parent = nullptr;
}

Node::Node(bool isObstacle)
{
    //Initialize Variables
    obstacle = isObstacle;
    visited = false;
    local_dist = 0;
    global_dist = 0;
    parent = nullptr;
}

v2f Node::getWorld()
{
    return world_pos;
}

void Node::setWorld(v2f world)
{
    world_pos = world;
}

v2u Node::getLocal()
{
    return local_pos;
}

void Node::setLocal(v2u local)
{
    local_pos = local;
}

AStar::AStar()
{
    //Initialize Variables
    grid_size = {50, 50};
    origin_pos = {0.0f, 0.0f};

    //Initialize Grid Nodes
    initGrid();
    DINFO("Completed Default A* Node Generation\n");
}

//Initialize a A* Grid at origin, with 4x the input size,
//at equal distance of tile_dim for each position
AStar::AStar(v2f origin, v2u grid_dim, v2f tile_dim)
{
    //Initialize Variables
    grid_size[0] = grid_dim[0] * 4;
    grid_size[1] = grid_dim[1] * 4;
    origin_pos = origin;
    v2f node_dim = {tile_dim[0] / 4.0f, tile_dim[1] / 4.0f};
    //Initialize Grid Nodes
    initGrid(node_dim);
    DINFO("Completed World A* Node Generation\n");
}

AStar::AStar(v2u size)
{
    //Initalize Variables
    grid_size = size;
    origin_pos = {0.0f, 0.0f};

    //Initalize Grid Nodes
    initGrid();
    DINFO("Completed V2U A* Node Generation\n");
}

AStar::AStar(u16 x_size, u16 y_size)
{
    //Initalize Variables
    grid_size[0] = x_size;
    grid_size[1] = y_size;
    origin_pos = {0.0f, 0.0f};

    //Initalize Grid Nodes
    initGrid();
    DINFO("Completed Coordinate A* Node Generation\n");
}

//Sets The Given Coordinate's Node to an Obstacle
void AStar::toggleObstacle(u16 x, u16 y)
{
    if (node_grid[x][y].obstacle) {
        node_grid[x][y].obstacle = false;
        DINFOF("Is Not an Obstacle");
    }
    node_grid[x][y].obstacle = true;
    DINFOF("Is an Obstacle");
}

//Returns The Node Grid's Size
v2u AStar::size()
{
    return grid_size;
}

//Retrieves The Node requested in the AStar
Node* AStar::getNode(u16 x, u16 y)
{
    if (x >= grid_size[0] && y >= grid_size[1]) {
        return nullptr;
    }
    return &node_grid[x][y];
}

//Set Node Positions & Conditionals
void AStar::initGrid(v2f dim)
{
    //Initialize Variables
    [[maybe_unused]] float offset = 0.0f;
    
    //Error & Halt Initialization if Any Dimension is 0 
    if ((dim[0] <= 0.0f) && (dim[1] <= 0.0f)) {
        DERROR("Dimensions of World Position Cannot Be Zero.");
        return;
    }

    //Resize The Node Grid
    node_grid.resize(grid_size[0]);
    for (u16 i = 0; i < node_grid.size(); i++) {
        node_grid[i].resize(grid_size[1]);
    }

    //Generate Nodes    
    for (u16 x = 0; x < grid_size[0]; x++) {
        for (u16 y = 0; y < grid_size[1]; y++) {
            node_grid[x][y].setLocal({x, y});
            node_grid[x][y].setWorld({
                ((float)x * dim[0]),
                ((float)y * dim[1])
            });
            node_grid[x][y].obstacle = false;
            node_grid[x][y].visited = false;
            node_grid[x][y].parent = nullptr;
        }
    }

    //Fill Each Node's Neighbors Vector Matrix
    genNeighbors();
}
}

void AStar::genNeighbors()
{
    //Load Each Node's neighbors Vector with all Adjacent Nodes
    for (u16 x = 0; x < grid_size[0]; x++) {
        for (u16 y = 0; y < grid_size[1]; y++) {
            
            //Bottom Neighbor
            if (y > 0) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x][y - 1]
                );
            }

            //Top Neighbor
            if (y < grid_size[1] - 1) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x][y + 1]
                );
            }
            
            //Left Neighbor
            if (x > 0) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x - 1][y]
                );
            }

            //Right Neighbor
            if (x < grid_size[0] - 1) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x + 1][y]
                );
            }
        }
    }
}

bool AStar::hasNeighbors(Node* node)
{
    return (!node->neighbors.empty());
}

Node* AStar::aStar(v2u begin_node, v2u ending_node)
{

    //Pointer to Start Node
    Node* start = &node_grid[begin_node[0]][begin_node[1]];

    //Pointer to Goal Node
    Node* goal = &node_grid[ending_node[0]][ending_node[1]];

    //Reset All Nodes to Default
    resetNodes();

    //Check If The Start Node has Neighbors
    if (!hasNeighbors(start)) {
        return nullptr;
    }

    //Initialize Start Node
    start->local_dist = 0.0f;
    start->global_dist = heuristics(start, goal);

    //Initialize Current Node to the Start Node
    Node* current = start;

    //Initialize an Ordered List of Untested Nodes
    std::list<Node*> untestedNodes;
    untestedNodes.push_back(start);

    //Primary Algorithm Loop
    while (!untestedNodes.empty()) {
        //Sort the list from least to greatest distance from the Goal
        untestedNodes.sort(
            //(This is a Lambda function, which is a unnamed function call)
            
            //If Equal to or Greater than, then swap positions
            [](const Node* a, const Node* b)
            {
                return a->global_dist < b->global_dist;
            }
        );
        
        //If the best Node has been visited, pop it from the list
        while ((!untestedNodes.empty()) && (untestedNodes.front()->visited)) {
            untestedNodes.pop_front();
        }

        //If the list is empty, break out of the loop
        if (untestedNodes.empty()) {
            return nullptr;
        }
        
        //Set The Current Node to the front of the list and set to visited
        current = untestedNodes.front();
        current->visited = true;

        //Check All Neighbors of The Current Node
        for (Node* neighbor : current->neighbors) {
            //Add Node to List if it's Not Been Visited and is Not a Obstacle
            if (!neighbor->visited && !neighbor->obstacle) {
                untestedNodes.push_back(neighbor);
            }

            //Generate a Potentially Lower Local Distance Based On Distance 
            //From current Node and The Selected neighbor
            //Plus current->local_dist
            float potential_low_goal = (
                current->local_dist + distance(current, neighbor)
            );

            //Set neigbor->parent to the current Node and
            //Set neighbor->local_dist to The Generated Value
            if (potential_low_goal > current->local_dist) {
                if ((neighbor->parent == nullptr) &&
                    (neighbor->parent != current) &&
                    (neighbor->parent != start)
                ) {
                    neighbor->parent = current;
                    neighbor->local_dist = potential_low_goal;

                    //The global_dist is a Measure of local_dist
                    //+ The Heuristic of neighbor Node and goal Node
                    neighbor->global_dist = (
                        neighbor->local_dist + heuristics(neighbor, goal)
                    );

                    //If Neighboring Node is Goal Node, Exit Algorithm
                    if (neighbor == goal) {
                        return current;
                    }
                }
            }
        } 
    }
    return nullptr;
}

void AStar::resetNodes()
{
    for (u16 x = 0; x < grid_size[0]; x++) {
        for (u16 y = 0; y < grid_size[1]; y++) {
            node_grid[x][y].visited = false;
            node_grid[x][y].global_dist = INFINITY;
            node_grid[x][y].local_dist = INFINITY;
            node_grid[x][y].parent = nullptr;
        }
    }
}

float AStar::distance(Node* a, Node* b)
{
    return sqrtf(
        (
            (a->getLocal()[0] - b->getLocal()[0])
            *
            (a->getLocal()[0] - b->getLocal()[0])
        ) 
        +
        (
            (a->getLocal()[1] - b->getLocal()[1])
            *
            (a->getLocal()[1] - b->getLocal()[1])
        )
    );
}

float AStar::heuristics(Node* a, Node* b)
{
    return distance(a, b);
}

//Enemy Generation
Enemies::Enemies()
{
    //Initialize Variables
    origin = {0.0f, 0.0f};
    amount = 1;
    timer = 5;
    health = 2.0f;
    damage = 1.0f;
    sprite_sheet = "player-front"; //Placeholder Texture !!

    //Resize The Entity Vector
    entities.resize(amount);

    //Initialize All Attributes
    entities[0] = ecs::ecs.entity().checkout();
    auto health = (
        ecs::ecs.component().assign<HEALTH>(entities[0])
    );
    auto transform = (
        ecs::ecs.component().assign<TRANSFORM>(entities[0])
    );
    auto sprite = (
        ecs::ecs.component().assign<SPRITE>(entities[0])
    );
    auto combat = (
        ecs::ecs.component().assign<COMBAT>(entities[0])
    );
    auto navigate = (
        ecs::ecs.component().assign<NAVIGATE>(entities[0])
    );

    //Set All Attributes
    health->max = (50.0f * health);
    health->hp = health->max;
    
    transform->pos = {
        0.0f + floatRand(10, 0),
        0.0f + floatRand(10, 0)
    };
    //Placeholder Values !!

    transform->rotation = 0.0f;
        
    sprite->ssheet = sprite_sheet;
    sprite->render_order = 0; //Placeholder Value !!

    combat->damage = (1.0f * this.damage);

    navigate->start = transform->pos;
    navigate->goal = {0.0f, 0.0f};
}

Enemies::Enemies(EnemyT type)
{
    //Initialize Variables
    switch (type)
    {
        case BANDIT:
            sprite_sheet = "player-front"; //Placeholder Texture !!
            break;
        case ALIEN:
            sprite_sheet = "player-front"; //Placeholder Texture !!
            break;
        case DEFAULT:
        default:
            sprite_sheet = "player-front"; //Placeholder Texture !!
            break;
    }
    origin = {0.0f, 0.0f};
    amount = 1;
    timer = 5;
    health = 2.0f;
    damage = 1.0f;

    //Resize The Entity Vector
    entities.resize(amount);

    //Initialize All Attributes
    entities[0] = ecs::ecs.entity().checkout();
    auto health = (
        ecs::ecs.component().assign<HEALTH>(entities[0])
    );
    auto transform = (
        ecs::ecs.component().assign<TRANSFORM>(entities[0])
    );
    auto sprite = (
        ecs::ecs.component().assign<SPRITE>(entities[0])
    );
    auto combat = (
        ecs::ecs.component().assign<COMBAT>(entities[0])
    );
    auto navigate = (
        ecs::ecs.component().assign<NAVIGATE>(entities[0])
    );

    //Set All Attributes
    health->max = (50.0f * health);
    health->hp = health->max;
    
    transform->pos = {
        0.0f + floatRand(10, 0),
        0.0f + floatRand(10, 0)
    };
    //Placeholder Values !!

    transform->rotation = 0.0f;
    
    sprite->ssheet = sprite_sheet;
    sprite->render_order = 0; //Placeholder Value !!

    combat->damage = (1.0f * this.damage);

    navigate->start = transform->pos;
    navigate->goal = {0.0f, 0.0f};
}

Enemies::Enemies(
    v2f origin,
    u16 number,
    u16 delay,
    float hp,
    float dmg,
    string ssheet
)
{
    //Initialize Variables
    this->origin = origin;
    amount = number;
    timer = delay;
    health = hp;
    damage = dmg;
    sprite_sheet = ssheet;

    //Check if Amount is 0
    if (amount <= 0) {
        DERROR("Amount of Enemies Generated Cannot be Zero.");
        return;
    }

    //Resize The Entity Vector
    entities.resize(amount);

    //Add All Attributes to Each Entity
    for (uint16_t i = 0; i < amount; i++) {
        entities[i] = ecs::ecs.entity().checkout();
        //Initialize All Attributes
        auto health = (
            ecs::ecs.component().assign<HEALTH>(entities[i])
        );
        auto transform = (
            ecs::ecs.component().assign<TRANSFORM>(entities[i])
        );
        auto sprite = (
            ecs::ecs.component().assign<SPRITE>(entities[i])
        );
        auto combat = (
            ecs::ecs.component().assign<COMBAT>(entities[i])
        );
        auto navigate = (
            ecs::ecs.component().assign<NAVIGATE>(entities[i])
        );

        //Set All Attributes
        health->max = (2.0f * this->health);
        health->hp = health->max;

        transform->pos = {
            0.0f + floatRand(10, 0),
            0.0f + floatRand(10, 0)
        }; 
        //Placeholder Values !!
        
        transform->rotation = 0.0f;
        
        sprite->ssheet = sprite_sheet;
        sprite->render_order = 0; //Placeholder Value !!

        combat->damage = (1.0f * this.damage);

        navigate->start = transform->pos;
        navigate->goal = {0.0f, 0.0f};
    }
}

Enemies::~Enemies()
{
    for (uint16_t i = 0; i < amount; i++)
    {
        ecs::ecs.entity().ret(entities[i]);
    }
    
}