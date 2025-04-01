#include "dchu.h"
#include <cmath>
#include <list>



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
    origin_pos[0] = origin[0];
    origin_pos[1] = origin[1];

    //Initialize Grid Nodes
    initGrid(tile_dim);
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
    if ((x >= grid_size[0] && y >= grid_size[1]) && (x < 0 && y < 0)) {
        return nullptr;
    }
    return &node_grid[x][y];
}

//Set Node Positions & Conditionals
void AStar::initGrid(v2f tile_dim)
{
    if ((tile_dim[0] > 0) && (tile_dim[1] > 0)) {
        DERRORF("Dimensions of Each World Position Cannot Be Zero.");
        return;
    }
    //Resize The Node Grid
    node_grid.resize(grid_size[0]);
    for (u16 i = 0; i < node_grid.size(); i++) {
        node_grid[i].resize(grid_size[i]);
    }

    //Generate Nodes
    for (u16 x = 0; x < grid_size[0]; x++) {
        for (u16 y = 0; y < grid_size[1]; y++) {
            node_grid[x][y].setLocal({x, y});
            node_grid[x][y].setWorld({
                ((float)x * tile_dim[0]),
                ((float)y * tile_dim[1])
            });
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
