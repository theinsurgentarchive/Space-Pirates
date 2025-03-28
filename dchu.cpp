#include "dchu.h"
#include <cmath>
#include <list>

bool isDisplayable(ecs::Entity* ent)
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

//A* Pathfinding Algorithm
//(INTEGRATION IN PROCESS)
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
    grid_size[0] = 10;
    grid_size[1] = 10;
    origin_pos[0] = 0.0f;
    origin_pos[1] = 0.0f;

    //Initialize Grid Nodes
    initGrid();
    DINFO("Completed Default A* Node Generation\n");
}

AStar::AStar(World& grid, v2f origin, v2u tile_dim)
{
    //Initialize Variables
    v2u grid_size;
    grid_size[0] = (grid.tiles().size() * 4);
    grid_size[1] = (grid.tiles()[0].size() * 4);
    node_grid.resize(grid_size[0]);
    for (uint16_t i = 0; i < node_grid.size(); i++) {
        node_grid[0].resize(grid_size[1]);
    }
    auto tiles = grid.tiles();
    v2f gen_dist = {
        tile_dim[0] / 4,
        tile_dim[1] / 4
    };
    v2f gen_dim = {
        tile_dim[0] / 2,
        tile_dim[1] / 2
    };
    origin_pos = origin;

    //Initialize Grid Nodes
    initGrid();

    //Set Each Node to an Obstacle if there is either no Tile or
    //The Tile is a Water Tile in The Current Position
    uint16_t x_iter = 0;
    uint16_t y_iter = 0;
    for (uint16_t x = 0; x < grid_size[0]; x++) {
        for (uint16_t y = 0; y < grid_size[1]; y++) {
            //Check if the Entity Has Both a Transform & a Sprite Component
            if (!isDisplayable(tiles[x][y])) {
                node_grid[x][y].obstacle = true;
                continue;
            }
            
            //Check If The Sprite Component Has a Water Texture
            //auto s = ecs::ecs.component().fetch<SPRITE>(tiles[x][y]);
            //if (s->ssheet == "./resources/textures/water.webp") {
            //    node_grid[x][y].obstacle = true;
            //    continue;
            //}

            //Set The World Position of Each Node to The Center of 
            //Each Quadrant of Each World Tile Using the Tile's World Position 
            auto t = ecs::ecs.component().fetch<TRANSFORM>(tiles[x][y]);
            v2f tile_world = t->pos;
            
            if (x_iter + 1 < grid_size[0] && y_iter + 1 < grid_size[1]){
                //Top-Left Quadrant
                v2f node_world = tile_world;
                node_world[0] -= gen_dist[0];
                node_world[1] -= gen_dist[1];
                node_grid[x_iter][y_iter].setWorld(node_world);

                //Top-Right Quadrant
                node_world = tile_world;
                node_world[0] += gen_dist[0];
                node_world[1] -= gen_dist[1];
                node_grid[x_iter + 1][y_iter].setWorld(node_world);

                //Bottom-Right Quadrant
                node_world = tile_world;
                node_world[0] -= gen_dist[0];
                node_world[1] += gen_dist[1];
                node_grid[x_iter + 1][y_iter + 1].setWorld(node_world);

                //Bottom-Left Quadrant
                node_world = tile_world;
                node_world[0] += gen_dist[0];
                node_world[1] += gen_dist[1];
                node_grid[x_iter][y_iter + 1].setWorld(node_world);
            }
            x_iter += 2;
        }
        y_iter += 2;
    }
    DINFO("Completed World Tiles A* Node Generation\n");
}
AStar::AStar(uint16_t x_size, uint16_t y_size)
{
    //Initalize Variables
    grid_size[0] = x_size;
    grid_size[1] = y_size;

    //Initalize Grid Nodes
    initGrid();
}

//Sets The Given Coordinate's Node to an Obstacle
void AStar::toggleObstacle(uint16_t x, uint16_t y)
{
    string info = "";
    if (node_grid[x][y].obstacle) {
        node_grid[x][y].obstacle = false;
        info += "Not an Obstacle.\n";
    }
    node_grid[x][y].obstacle = true;
    info += "an Obstacle.\n";
    DINFOF("Node of (%d,%d) is %s", x, y, info);
}

//Returns The Node Grid's Size
v2u AStar::size()
{
    return grid_size;
}

//Retrieves The Node requested in the AStar
Node* AStar::getNode(uint16_t x, uint16_t y)
{
    if ((x <= grid_size[0]) && (y <= grid_size[1])) {
        return &node_grid[x][y];
    }
}

//Set Node Positions & Conditionals
void AStar::initGrid()
{
    //Resize The Node Grid
    node_grid.resize(grid_size[0]);
    for (uint16_t i = 0; i < node_grid.size(); i++) {
        node_grid[i].resize(grid_size[i]);
    }

    //Generate Nodes
    for (uint16_t x = 0; x < grid_size[0]; x++) {
        for (uint16_t y = 0; y < grid_size[1]; y++) {
            node_grid[x][y].setLocal({x, y});
            node_grid[x][y].setWorld({(float)x, (float)y});
            node_grid[x][y].obstacle = false;
            node_grid[x][y].visited = false;
            node_grid[x][y].parent = nullptr;
        }
    }

    //Fill Each Node's Neighbors Vector Matrix
    genNeighbors();
}

void AStar::genNeighbors()
{
    //Load Each Node's neighbors Vector with all Adjacent Nodes
    for (uint16_t x = 0; x < grid_size[0]; x++) {
        for (uint16_t y = 0; y < grid_size[1]; y++) {
            
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

Node* AStar::aStar(uint16_t begin_node[], uint16_t ending_node[])
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
    for (uint16_t x = 0; x < grid_size[0]; x++) {
        for (uint16_t y = 0; y < grid_size[1]; y++) {
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
/*
//Inventory Class Functions
Inventory::Inventory()
{
    initStoreVolume(3, 4);
    full = false;
}

Inventory::~Inventory()
{
    //Delete through all rows in the matrix
    for (uint16_t i = 0; i < inv_size[0]; i++) {
        delete [] storage[i];
    }

    //Delete Storage Matrix
    delete [] storage;
}

void Inventory::initStoreVolume(uint16_t x, uint16_t y)
{
    //Initialize Variables
    inv_size = {static_cast<uint16_t>(x),static_cast<uint16_t>(y)};
    //Initialize Inventory Matrix
    storage = new item*[inv_size[0]];
    for (uint16_t i = 0; i < inv_size[0]; i++) {
        storage[i] = new item[inv_size[1]];
    }

    //Pre-Load Item Slots
    if (inv_size[0] > 0 && inv_size[1] > 0) {
        uint16_t counter = 0;
        for (uint16_t i = 0; i < inv_size[0]; i++) {
            for (uint16_t j = 0; j < inv_size[1]; j++) {
                storage[i][j].item_name = ("Slot " + std::to_string(counter));
                storage[i][j].slot_num = ++counter;
            }
        }
    }
    //If DEVMODE, Run Unit Test
    #ifdef DEBUG
        std::string test = "testItem";
        bool flag = false;
        for (uint16_t i = 0; i < inv_size[0]; i++) {
            for (uint16_t j = 0; j < inv_size[1]; j++) {
                storage[i][j].item_name = test;
            }
            
        }
        for (uint16_t i = 0; i < inv_size[0]; i++) {
            for (uint16_t j = 0; j < inv_size[1]; j++) {
                if(storage[i][j].item_name != test) {
                    flag = true;
                    break;
                }
            }
            if(flag) {
                break;
            }
        }
        if (!flag) {
            for (uint16_t i = 0; i < inv_size[0]; i++) {
                for (uint16_t j = 0; j < inv_size[1]; j++) {
                    std::cout << storage[i][j].item_name << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "Inventory Matrix - OK\n\n";
        } else {
            std::cout << "ERROR, Inventory Matrix not Loading";
        }
    #endif
}
//ECS: Inventory Management System
//namespace ecs
//{
//    InventorySystem::InventorySystem()
//    {
//
//    }
//
//    void InventorySystem::update([[maybe_unused]]float dt)
//    {
//
//    }
//}
*/