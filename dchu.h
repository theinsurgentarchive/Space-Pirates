#pragma once
#include "jlo.h"
#include "jsandoval.h"

//Define Alias for ecs Components
#define COMBAT ecs::Combat
#define NAVIGATE ecs::Navigate

//Game Over Screen
void renderGameOver(v2u);

//Can The Given Entity be Rendered?
bool canRender(ecs::Entity*);

//Return a Float that Represents the Distance Between 2 Vectors
float v2fDist(v2f, v2f);

//Returns Unit Vector representing the Direction of The Vector
//(Used to Calculate The Change in XY of an Entity)
v2f v2fNormal(v2f);

//Generate a Randomized Float Number Between 2 Integers
float floatRand(int16_t, int16_t);

//A* Pathfinding Nodes, Connects The Grid
class Node
{
    private:
        //Positional Variables
        v2f world_pos;
        v2u local_pos;
    public:

        //Variables
        bool obstacle, visited;
        float local_dist, global_dist;
        std::vector<Node*> neighbors;
        
        //Node Directly Preceding & Proceeding Current Node
        Node* parent {nullptr};
        //Constructor
        Node();
        Node(bool);

        //Function
        v2f getWorld();
        void setWorld(v2f);
        v2u getLocal();
        void setLocal(v2u);
};

namespace ecs
{
    class Navigate
    {
        private:
            std::vector<v2f> nodes;
            u16 current;
            ecs::Entity* self;
        public:
            //Constructor
            Navigate();

            //Function
            v2f nodePos();
            void genPath(Node*);
            void reset();
            void moveToCurrent();
    };
}


//AStarGrid of Node Elements, Used in A* Search
class AStar
{
    private:
        //AStar's X & Y Axis Size
        v2u grid_size;

        //The Position in The World that The Grid is Generated From.
        v2f origin_pos;
    public:
        //Dynamic Node Grid
        std::vector<std::vector<Node>> node_grid;

        //Constructor
        AStar();
        AStar(v2f, v2u, v2f);
        AStar(v2u);
        AStar(u16, u16);

        //Sets a Node to an Obstacle in A*
        void toggleObstacle(u16, u16);

        //Get The Node Grid's Size
        v2u size();

        Node* getNode(u16, u16);

        void initGrid(v2f dim = {1.0f, 1.0f});

        void genNeighbors();
        bool hasNeighbors(Node*);

        //A* Search Algorithm
        Node* aStar(v2u, v2u);

        void resetNodes();

        //Calculates The Distance From One Node to The Next
        float distance(Node*, Node*);

        //Generates Biased Data Based On Two Given Input Nodes
        float heuristics(Node*, Node*);
};

//Move an Entity to a Position or Entity with a Transform
void moveTo(ecs::Entity*, v2f);
void moveTo(ecs::Entity*, ecs::Entity*);

//Enemy Generation
enum EnemyT 
{
    DEFAULT,  //0
    BANDIT,   //1
    ALIEN     //2
};

class Enemy
{
    private:
        ecs::Entity* ent;
        u16 atk_Timer;
        u16 path_Timer;
        bool can_damage;
    public:
        //Constructor
        Enemy(ecs::Entity*);
        Enemy(ecs::Entity*, v2f);
        
        //Function
        void initEnemy();
        void loadEnemyTex(
            std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>& ssheets
        );
        bool doDamage(ecs::Entity*, ecs::Entity*);
        void action();

        //Getter
        u16 getAtkTimer();
        u16 getPathTimer();
        bool getCanDamage();

};