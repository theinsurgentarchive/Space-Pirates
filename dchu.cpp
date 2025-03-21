#include "dchu.h"
#include <cmath>
#include <list>

//A* Pathfinding Algorithm
//(NEEDS TO BE UPDATED TO MATCH AND INTEGRATE WITH PROJECT)
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

AStarGrid::AStarGrid()
{
    //Initialize Variables
    grid_size[0] = 10;
    grid_size[1] = 10;

    //Initialize Grid Nodes
    initGrid();
}

AStarGrid::AStarGrid(std::vector<std::vector<Node>> grid)
{
    //Set this.node_grid to The Passed Grid
    node_grid = grid;

    //Initialize Variables
    grid_size[0] = sizeof(grid) / sizeof(grid[0]);
    grid_size[1] = sizeof(grid[0]) / sizeof(grid[0][0]);
}

AStarGrid::AStarGrid(uint16_t x_size, uint16_t y_size)
{
    //Initalize Variables
    grid_size[0] = x_size;
    grid_size[1] = y_size;

    //Initalize Grid Nodes
    initGrid();
}

//Sets The Given Coordinate's Node to an Obstacle
void AStarGrid::setObstacle(uint16_t x, uint16_t y)
{
    node_grid[x][y].obstacle = true;
}

//Returns The Node Grid's X-Axis Size
uint16_t AStarGrid::getSizeX()
{
    return grid_size[0];
}

//Returns The Node Grid's Y-Axis Size
uint16_t AStarGrid::getSizeY()
{
    return grid_size[1];
}

//Retrieves The Node requested in the AStarGrid
Node* AStarGrid::getNode(uint16_t x, uint16_t y)
{
    if ((x <= grid_size[0]) && (y <= grid_size[1])) {
        return &node_grid[x][y];
    }
}

//Generate Node AStarGrid, Set Node Positions
void AStarGrid::initGrid()
{
    //Resize The Node Grid
    node_grid.resize(grid_size[0]);
    for (uint16_t i = 0; i < node_grid.size(); i++) {
        node_grid[i].resize(grid_size[i]);
    }

    //Generate Nodes
    for (uint16_t x = 0; x < grid_size[0]; x++) {
        for (uint16_t y = 0; y < grid_size[1]; y++) {
            node_grid[x][y].x = x;
            node_grid[x][y].y = y;
            node_grid[x][y].obstacle = false;
            node_grid[x][y].visited = false;
            node_grid[x][y].parent = nullptr;
        }
    }

    //Fill Each Node's Neighbors Vector Matrix
    genNeighbors();
}

void AStarGrid::genNeighbors()
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
            
            //Bottom-Left Neighbor
            if (x > 0 && y > 0) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x - 1][y - 1]
                );
            }

            //Top-Left Neighbor
            if (x > 0 && y < grid_size[1] - 1) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x - 1][y + 1]
                );
            }

            //Bottom-Right Neighbor
            if (x < grid_size[0] - 1 && y > 0) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x + 1][y - 1]
                );
            }

            //Top-Right Neighbor
            if (x < grid_size[0] - 1 && y < grid_size[1] - 1) {
                node_grid[x][y].neighbors.push_back(
                    &node_grid[x + 1][y + 1]
                );
            }
        }
    }
}

bool AStarGrid::hasNeighbors(Node* node)
{
    return (node->neighbors.empty() == false);
}

void AStarGrid::aStar(uint16_t begin_node[], uint16_t ending_node[])
{

    //Pointer to Start Node
    Node* start = &node_grid[begin_node[0]][begin_node[1]];

    //Pointer to Goal Node
    Node* goal = &node_grid[ending_node[0]][ending_node[1]];

    //Reset All Nodes to Default
    resetNodes();

    //Check If The Start Node has Neighbors
    if (!hasNeighbors(start)) {
        return;
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
            break;
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
                        return;
                    }
                }
            }
        } 
    }
}

void AStarGrid::resetNodes()
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

float AStarGrid::distance(Node* a, Node* b)
{
    return sqrtf(
        ((a->x - b->x) * ((a->x - b->x))) + ((a->y - b->y) * (a->y - b->y))
    );
}

float AStarGrid::heuristics(Node* a, Node* b)
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