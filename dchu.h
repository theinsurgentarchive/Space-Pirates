#pragma once
#include "jlo.h"
#include "jsandoval.h"

//Define Alias for ecs Components
#define COMBAT ecs::Combat
#define NAVIGATE ecs::Navigate

//Load Splash Screen
void loadSplash(std::unordered_map<std::string, std::shared_ptr<SpriteSheet>>&);

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


//AStar Grid of Node Elements, Used in A* Search
class AStar
{
    private:
        //AStar's X & Y Axis Size
        v2u grid_size;

        //The Position in The World that The Grid is Generated From.
        v2f origin_pos;

        //The World Position Step
        v2f origin_step;
    public:
        //Dynamic Node Grid
        std::vector<std::vector<Node>> node_grid;

        //Constructor
        AStar();
        AStar(v2f, v2u, v2f);
        AStar(v2u);
        AStar(u16, u16);


        //Sets Node(s) to an Obstacle in A*
        void toggleObstacle(u16, u16);
        void setObstacles(World*);

        //Get The Node Grid's Size
        v2u size();

        //Get The Distance between the Center of Nodes
        v2f getStep();

        //Retrieve Nodes
        Node* getNode(u16, u16);
        Node* findClosestNode(v2f);

        void initGrid(v2f dim = {1.0f, 1.0f});

        //Neighbor Related Function
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

namespace ecs
{
    class Navigate
    {
        private:
            std::vector<Node*> nodes;
            
            //Get The Current Position Within The Node Vector
            u16 current_node_pos;

            //Reference to world grid
            AStar* grid;

            //Has The Path Been Fully Traversed?
            bool finished;
        public:
            //Constructor
            Navigate();

            //Function
            float* nodePos();
            void genPath(Node*, Node*);
            void reset();
            bool nextNode();

            //Getter
            AStar* getAStar();
            bool getStatus();

            //Setter
            void setAStar(AStar*);
            void setStatus(bool);
    };
}

//Move an Entity to a Position or Entity with a Transform
void moveTo(ecs::Entity*, v2f);
void moveTo(ecs::Entity*, ecs::Entity*);
void moveTo(ecs::Entity*, Node*);
//Enemy Generation
enum EnemyT 
{
    DEFAULT,  //0
    BANDIT,   //1
    ALIEN     //2
};

void loadEnemyTex(
    std::unordered_map<std::string,std::shared_ptr<SpriteSheet>>&
);

class Enemy
{
    private:
        ecs::Entity* ent;
        u16 atk_Timer;
        u16 path_Timer;
        bool can_damage;
        bool can_gen_path;
    public:
        //Constructor
        Enemy(ecs::Entity*);
        Enemy(ecs::Entity*, v2f t_mod);
        
        //Function
        void initEnemy();
        bool doDamage(ecs::Entity*, ecs::Entity*);
        void action(World*);

        //Getter
        u16 getAtkTimer();
        u16 getPathTimer();
        bool getCanDamage();
        bool getCanGenPath();

};