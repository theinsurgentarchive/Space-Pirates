#include "dchu.h"
#include <cmath>
#include <list>
#include <cstdlib>
#include <string>
#include <ctime>

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
            ent->id
        );
    }
    return display;
}

//Calcuate The Distance Between 2 Vectors
float v2fDist(v2f v1, v2f v2)
{
    return sqrtf(
        ((v1[0] - v2[0]) * (v1[0] - v2[0])) +
        ((v1[1] - v2[1]) * (v1[1] - v2[1]))
    );
}

//Generate a Normal from a Vector
v2f v2fNormal(v2f vec)
{
    //Calculate The Magnitude of The Vector
    float mag = sqrtf((vec[0] * vec[0]) + (vec[1] * vec[1]));
    
    //Calculate & Return The Normalized Vector
    v2f norm {(vec[0] / mag), (vec[1] / mag)};
    return norm;
}

float floatRand(int16_t max, int16_t min)
{
    //Initialize Variables
    int16_t d_max = 99;
    int16_t d_min = 0;
    float random = 0.0f;

    //Generate Whole Number
    int16_t whole = (rand() % (max - min + 1) - min);
    if (whole >= max) {
        return (float)max;
    }

    //Generate Decimal
    int16_t decimal = (rand() % (d_max - d_min + 1) - d_min);
    
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
                ((float)x * dim[0] + offset),
                ((float)y * dim[1] + offset)
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

    resetNodes();
    if (!hasNeighbors(start)) {
        return nullptr;
    }

    //Initialize Start Node
    start->local_dist = 0.0f;
    start->global_dist = heuristics(start, goal);

    //Initialize Current Node
    Node* current = start;

    //Initialize an Ordered List of Untested Nodes
    std::list<Node*> untestedNodes;
    untestedNodes.push_back(start);

    //Primary Algorithm Loop
    while (!untestedNodes.empty()) {
        
        //Sort The List From least to greatest distance from the Goal
        untestedNodes.sort(
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
        
        //Set The Current Node to the front of the list & set to visited
        current = untestedNodes.front();
        current->visited = true;

        //Check All Neighbors of The Current Node
        for (Node* neighbor : current->neighbors) {
            //Add Node to List if it's Not Been Visited & is Not a Obstacle
            if (!neighbor->visited && !neighbor->obstacle) {
                untestedNodes.push_back(neighbor);
            }

            //Generate a Potentially Lower Local Distance Based On Distance 
            float potential_low_goal = (
                current->local_dist + distance(current, neighbor)
            );

            //Set neighbor->parent to the current Node &
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

//Find The Distance between Two Points
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

void initEnemy(ecs::Entity* foe)
{
    //Initialize Components
    auto health = ecs::ecs.component().assign<HEALTH>(foe);
    auto sprite = ecs::ecs.component().assign<SPRITE>(foe);
    auto transform = ecs::ecs.component().assign<TRANSFORM>(foe);
    auto physics = ecs::ecs.component().assign<PHYSICS>(foe);
    auto navigate = ecs::ecs.component().assign<NAVIGATE>(foe);

    //Set Component Variables
    health->max = 50.0f;
    health->health = health->max;
    sprite->ssheet = "placeholder";
    sprite->render_order = 14;
    transform->pos = {0.0f, 0.0f};
    physics->acc = {0.0f, 0.0f};
    physics->vel = {0.0f, 0.0f};
}

void moveTo(ecs::Entity* ent, v2f target)
{
    auto physics = ecs::ecs.component().fetch<PHYSICS>(ent);
    auto transform = ecs::ecs.component().fetch<TRANSFORM>(ent);
    v2f dif {target[0] - transform->pos[0], target[1] - transform->pos[1]};
    //if The Difference is Within 0.5 Error Zero Velocity & Accel, & Return
    if ((dif[0] < 0.5f && dif[0] > -0.5f) &&
        (dif[1] < 0.5f && dif[1] > -0.5f)
    ) {
        physics->acc = {0.0f, 0.0f};
        physics->vel = {0.0f, 0.0f};
        return;
    }

    //Set Acceleration
    float accel = 50.0f;
    float dist = v2fDist(target, transform->pos);
    float reduce = 1.0f;
    if (dist < accel) {
        reduce = dist / accel;
    }
    v2f dir = v2fNormal(dif);
    v2f move {((accel * dir[0]) * reduce), ((accel * dir[1]) * reduce)};
    physics->acc[0] = move[0];
    physics->acc[1] = move[1];

    //Set Acceleration to 0 if The Entity Axis is Within Target
    if (
        (transform->pos[0] > target[0] - move[0] - 0.1f) &&
        (transform->pos[0] < target[0] + move[0] + 0.1f)
    ) {
        DINFOF("%d Entity within Target X Range.\n", ent->id);
        physics->acc[0] = 0.0f;
        physics->vel[0] *= 0.9f;
    }

    if (
        (transform->pos[1] > target[1] - move[1] - 0.1f) &&
        (transform->pos[1] < target[1] + move[1] + 0.1f)
    ) {
        DINFOF("%d Entity within Target Y Range.\n", ent->id);
        physics->acc[1] = 0.0f;
        physics->vel[1] *= 0.9f;
    }

    //Speed Limit
    float top_speed = 50.0f;
    if (physics->vel[0] > top_speed) {
        physics->vel[0] = top_speed;
        physics->acc[0] = 0.0f;
    }
    if (physics->vel[0] < -top_speed) {
        physics->vel[0] = -top_speed;
        physics->acc[0] = 0.0f;
    }
    if (physics->vel[1] > top_speed) {
        physics->vel[1] = top_speed;
        physics->acc[1] = 0.0f;
    }
    if (physics->vel[1] < -top_speed) {
        physics->vel[1] = -top_speed;
        physics->acc[1] = 0.0f;
    }
}

void moveTo(ecs::Entity* ent, ecs::Entity* target)
{
    auto tar = ecs::ecs.component().fetch<TRANSFORM>(target);
    if (tar == nullptr) {
        DWARNF(
            "Error, entity %d does not have a transform component\n",
            target->id
        );
        return;
    }
    moveTo(ent, tar->pos);
}

/*
void loadEnemyTex(
    std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets
)
{}
*/
ecs::Navigate::Navigate()
{
    current = 0;
}

v2f ecs::Navigate::nodePos()
{
    if (current >= nodes.size()) {
        DWARN("Path Overshoot, Returning Default Value.\n");
        return {0.0f, 0.0f};
    }
    return nodes[current];
}

void ecs::Navigate::genPath()
{
    reset();
}

void ecs::Navigate::reset()
{
    current = 0;
    nodes.clear();
}