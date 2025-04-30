#include "dchu.h"
#include <cmath>
#include <list>
#include <cstdlib>
#include <string>
#include <ctime>
#include <unistd.h>

extern ecs::Entity* player;

//Render Game Over Screen
void renderGameOver(v2u res)
{
    sleep(2);
}

//Renderability Check
bool canRender(ecs::Entity* ent)
{
    bool display = true;
    //Check entity for Transform & Sprite Component
    if (!ecs::ecs.component().has<TRANSFORM,SPRITE>(ent)) {
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
    if (this == nullptr) {
        DERRORF("Attempting to Call getLocal Without Existing Returning 0, 0");
        return {0, 0};
    }
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
    grid_size[0] = grid_dim[0];
    grid_size[1] = grid_dim[1];
    origin_pos = origin;
    origin_step = {tile_dim[0], tile_dim[1]};
    //Initialize Grid Nodes
    initGrid(origin_step);
    DINFO("Completed World A* Node Generation\n");
}

AStar::AStar(v2u size)
{
    //Initalize Variables
    grid_size = size;
    origin_pos = {0.0f, 0.0f};
    origin_step = {0, 0};

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
    origin_step = {0, 0};

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
    if (a == nullptr) {
        DERRORF("%s does not exist.", a);
        std::cout << "An Error has occurred\n";
        return 0.0f;
    }
    if (b == nullptr) {
        DERRORF("%s does not exist.", b);
        std::cout << "An Error has occurred\n";
        return 0.0f;
    }
    return sqrtf(
        (
            ((float)a->getLocal()[0] - (float)b->getLocal()[0])
            *
            ((float)a->getLocal()[0] - (float)b->getLocal()[0])
        ) 
        +
        (
            ((float)a->getLocal()[1] - (float)b->getLocal()[1])
            *
            ((float)a->getLocal()[1] - (float)b->getLocal()[1])
        )
    );
}

float AStar::heuristics(Node* a, Node* b)
{
    return distance(a, b);
}


//Retrieves The Node requested in the AStar
Node* AStar::getNode(u16 x, u16 y)
{
    if ((x >= grid_size[0] || y >= grid_size[1]) || (x < 0 || y < 0)) {
        DWARN("Cannot find Node, Out of Bounds\n");
        return nullptr;
    }
    return &node_grid[x][y];
}

Node* AStar::findClosestNode(v2f pos)
{
    v2f find {floor(pos[0]), floor(pos[1])};
    v2f select {
        round(find[0] / origin_step[0]),
        round(find[1] / origin_step[1])
    };

    //Get Node
    Node* result = getNode(select[0], select[1]);
    return result;
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
    u16 iter = 0;
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
    DINFO("Finished Initializing AStar Grid\n");
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
    Node* start = getNode(begin_node[0], begin_node[1]);
    std::cout << start << std::endl;
    if (start == nullptr) {
        DERROR("Start Node Failed to initialize");
        return nullptr;
    }
    
    //Pointer to Goal Node
    Node* goal = getNode(ending_node[0],ending_node[1]);
    std::cout << goal << std::endl;
    if (goal == nullptr) {
        DERROR("Goal Node Failed to initialize");
        return nullptr;
    }

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

void moveTo(ecs::Entity* ent, v2f target)
{
    auto [physics, transform] = 
                            ecs::ecs.component().fetch<PHYSICS, TRANSFORM>(ent);
    v2f dif {target[0] - transform->pos[0], target[1] - transform->pos[1]};
    //if The Difference is Within 0.3 Error Zero Velocity & Accel, & Return
    if ((dif[0] < 0.3f && dif[0] > -0.3f) &&
        (dif[1] < 0.3f && dif[1] > -0.3f)
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
    physics->vel[0] = move[0];
    physics->vel[1] = move[1];

    //Set Acceleration to 0 if The Entity Axis is Within Target
    if (
        (transform->pos[0] > target[0] - move[0] - 0.1f) &&
        (transform->pos[0] < target[0] + move[0] + 0.1f)
    ) {
        DINFOF("%d Entity within Target X Range.\n", ent->id);
        physics->acc[0] = 0.0f;
        physics->vel[0] *= 0.99f;
    }

    if (
        (transform->pos[1] > target[1] - move[1] - 0.1f) &&
        (transform->pos[1] < target[1] + move[1] + 0.1f)
    ) {
        DINFOF("%d Entity within Target Y Range.\n", ent->id);
        physics->acc[1] = 0.0f;
        physics->vel[1] *= 0.99f;
    }
    /*
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
    */
}

void moveTo(ecs::Entity* ent, ecs::Entity* target)
{
    auto [tar] = ecs::ecs.component().fetch<TRANSFORM>(target);
    if (tar == nullptr) {
        DWARNF(
            "Error, entity %d does not have a transform component\n",
            target->id
        );
        return;
    }
    moveTo(ent, tar->pos);
}

Enemy::Enemy(ecs::Entity* ent) : Enemy(ent, {0.07f, 6.0f})
{}

Enemy::Enemy(ecs::Entity* ent, v2f t_mod)
{
    atk_Timer = (u16)(t_mod[0] * 1000.0f);
    path_Timer = (u16)(t_mod[1] * 1000.0f);
    can_damage = true;
    std::cout << can_damage << std::endl << std::flush;
    this->ent = ent;
    initEnemy();
}

void Enemy::initEnemy()
{
    //Initialize Components
    auto [health, collide, sprite, transform, physics, navigate] = 
    ecs::ecs.component().assign
                <HEALTH, COLLIDER, SPRITE, TRANSFORM, PHYSICS, NAVIGATE>(ent);

    //Set Component Variables
    health->max = 50.0f;
    health->health = health->max;
    sprite->ssheet = "placeholder";
    sprite->render_order = 14;
    transform->pos = {1000.0f, 1000.0f};
    physics->acc = {0.0f, 0.0f};
    physics->vel = {0.0f, 0.0f};
    collide->passable = true;
    collide->dim = {16, 16};
}
/*
void Enemy::loadEnemyTex(
    std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets
)
{}
*/
bool Enemy::doDamage(ecs::Entity* ent, ecs::Entity* ent2)
{
    auto [p_collide, p_transform, health] = ecs::ecs.component().fetch
                                        <COLLIDER, TRANSFORM, HEALTH>(ent2);
    auto [s_collide, s_transform] = ecs::ecs.component().fetch
                                                    <COLLIDER, TRANSFORM>(ent);
    if (collided(p_transform, s_transform, p_collide, s_collide)) {
        if ((health->health > 0)){
            health->health -= 1;
        }
        return true;
    }
    return false;
}

void Enemy::action()
{
    auto [navi, trav] = ecs::ecs.component().fetch<NAVIGATE, TRANSFORM>(ent);
    auto [tran] = ecs::ecs.component().fetch<TRANSFORM>(player);
    static std::chrono::high_resolution_clock::time_point last_time;
    v2f node_pos = navi->nodePos();

    //Check if The Enemy is Due for another A* Pass.
    if (can_gen_path) {
        navi->genPath(
            navi->getAStar()->findClosestNode(tran->pos),
            navi->getAStar()->findClosestNode(trav->pos)
        );
        can_gen_path = false;
    } else {
        auto current = std::chrono::high_resolution_clock::now();
        auto t_elasped = std::chrono::duration_cast<std::chrono::milliseconds>(
            current - last_time
        );
        if (t_elasped.count() >= path_Timer) {
            can_gen_path = true;
        }
    }

    //Move Towards Next Node in The Path, Otherwise Move Towards The Player
    if (!(
        (node_pos[0] < (trav->pos[0] + 0.5f)) &&
        (node_pos[0] > (trav->pos[0] - 0.5f)) &&
        (node_pos[1] < (trav->pos[1] + 0.5f)) &&
        (node_pos[1] > (trav->pos[1] - 0.5f))
    )) {
        moveTo(ent, node_pos);
    } else {
        if (!navi->nextNode()) {
            moveTo(ent, player);
        }
    }
    //Check if The Enemy has Hit The Player
    if (can_damage) {
        if(doDamage(ent, player)) {
            last_time = std::chrono::high_resolution_clock::now();
            can_damage = false;
        }
    } else {
        auto current = std::chrono::high_resolution_clock::now();
        auto t_elasped = std::chrono::duration_cast<std::chrono::milliseconds>(
            current - last_time
        );
        if (t_elasped.count() >= atk_Timer) {
            can_damage = true;
        }
    }
}


u16 Enemy::getAtkTimer()
{
    return atk_Timer;
}

u16 Enemy::getPathTimer()
{
    return path_Timer;
}

bool Enemy::getCanDamage()
{
    return can_damage;
}

bool Enemy::getCanGenPath()
{
    return can_gen_path;
}

ecs::Navigate::Navigate()
{
    current_node_pos = 0;
    grid = nullptr;
}

v2f ecs::Navigate::nodePos()
{
    if (current_node_pos >= nodes.size()) {
        DWARN("Path Overshoot, Returning Default Value.\n");
        return {-0.0000000000009f, 0.0000000000009f};
    }
    return nodes[current_node_pos]->getWorld();
}

void ecs::Navigate::reset()
{
    current_node_pos = 0;
    nodes.clear();
}

void ecs::Navigate::genPath(Node* start, Node* end)
{
    reset();
    Node* start_node = start;
    Node* goal_node = end;
    grid->aStar(start->getLocal(), end->getLocal());
    Node* current = goal_node;
    std::vector<Node*> temp;
    while (current != nullptr && current != start_node) {
        if (current != goal_node) {
            nodes.push_back(current);
        }
        current = current->parent;
        std::cout << current << std::endl;
    }
    DINFO("Finished Generating Path.\n");
}

bool ecs::Navigate::nextNode()
{
    if (current_node_pos < nodes.size()) {
        current_node_pos++;
        return true;
    } else {
        return false;
    }
}

AStar* ecs::Navigate::getAStar()
{
    return grid;
}

void ecs::Navigate::setAStar(AStar* astar)
{
    grid = astar;
}