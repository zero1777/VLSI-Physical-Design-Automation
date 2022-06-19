#include <iostream>
#include <string>
#include <vector>
#include "Reader.hpp"

using namespace std;

class Node;
class Shape;

class Floorplan {
public: 
    Input *input;
    vector<int> intialNPE;
    int range;
    int totalArea;
    vector<Node *> HBNodes, VHNodes;

    Floorplan(Input *ip);

    void debugBallot(vector<int> _n);

    void initialConstructNodes();
    void initialFloorplan();

    void chainInvert(vector<int> &npe, int pos);
    bool checkIfSkewed(vector<int> &npe, int pos);
    bool checkIfBalloting(vector<int> &npe, int pos);
    vector<int> perturbation(vector<int> npe, int type);
    void stockMeyer(Node *node);
    Node *constructTreeViaPostOrder(vector<int>& npe);
    int calculateHPWL(Net *net);
    int calculateWireLength();
    int calculateCostFunc(vector<int>& npe, int whichStage);
    void setHardBlockCoord(Node *node, int coordX, int coordY, int whichOne);

    vector<int> SA_Floorplanning(vector<int> &inputNPE, double e, double r, int k, int whichStage);

    void Algorithm();

    // output
    void outputResult(string filename);

    // testing
    void test();

    // debug
    vector<int> saProcess(double const &c, double const &r, int const &k,
                                      std::vector<int> const &NPE, bool const &focusWirelength);
    std::vector<int> perturbNPE(std::vector<int> npe, int const &type);
    int calCost(std::vector<int> &npe, bool const &focusWirelength);
    bool checkSkewed(std::vector<int> const &npe, size_t const &pos);

};

class Node {
public:
    int kind;

    HardBlock *hardblock;
    Node *left, *right;
    vector<Shape> possibleShape;
    
    
    Node(int k, HardBlock *hb);
};

class Shape {
public:
    int width, height;
    int leftShape, rightShape;
    Shape(int w, int h, int l, int r);
};