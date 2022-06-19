#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>

#include <climits>
#include <cstdlib>
#include <ctime>
#include <stack>
#include <math.h>
#include <time.h>

using namespace std;

#define B 0
#define V -1
#define H -2

class Pin;
class HardBlock;
class Net;

class Input {
public:
    vector<HardBlock *> hardblocks;
    vector<Net *> nets;
    unordered_map<string, Pin *> namePinMap;
    int numHRB;
    int numTerm;
    double deadSpaceRatio;

    void ReadAllFile(char *argv[]);
    void ReadHardBlocks(string filename);
    void ReadNets(string filename);
    void ReadPl(string filename);
};

class Pin {
public: 
    string name;
    int x, y; // coordinate
    Pin(string name, int x, int y);
};

class HardBlock {
public:
    string name;
    int width, height;
    int x, y;
    bool isRotated;
    Pin *pin;
    HardBlock(string n, int w, int h);
    void setValue(int X, int Y, int W, int H_);
};

class Net {
public:
    vector<Pin *> pins;
};

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
    int calculateCostFunc(vector<int>& npe);
    void setHardBlockCoord(Node *node, int coordX, int coordY, int whichOne);

    vector<int> SA_Floorplanning(double e, double r, int k);

    void Algorithm();

    // output
    void outputResult(string filename);

    // testing
    void test();
    

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

void Input::ReadAllFile(char *argv[]) {
    ReadHardBlocks(argv[1]);
    ReadPl(argv[3]);
    ReadNets(argv[2]);
    deadSpaceRatio = stod(argv[5]);
}

void Input::ReadHardBlocks(string filename) {
    // NumHardRectilinearBlocks : 100
    // NumTerminals : 334
    // sb0 hardrectilinear 4 (0, 0) (0, 33) (43, 33) (43, 0)
    // p1 terminal

    ifstream hardblockfile(filename);
    string str;
    while (getline(hardblockfile, str)) {
        if (str.empty()) continue;
        stringstream ss(str);
        string first, second;
        ss >> first;
        ss >> second;

        if (first == "NumHardRectilinearBlocks") {
            ss >> numHRB;
        }
        else if (first == "NumTerminals") {
            ss >> numTerm;
        }
        else {
            if (second == "hardrectilinear") {
                string coord;
                int x[4], y[4];
                getline(ss, coord);
                sscanf(coord.c_str(), " 4 (%d, %d) (%d, %d) (%d, %d) (%d, %d)", &x[0], &y[0], &x[1], &y[1], &x[2], &y[2], &x[3], &y[3]);
                auto arr = {x[0], x[1], x[2], x[3]};
                int width = max(arr) - min(arr);
                auto arr2 = {y[0], y[1], y[2], y[3]};
                int height = max(arr2) - min(arr2);

                HardBlock *newHardBlock = new HardBlock(first, width, height);
                hardblocks.push_back(newHardBlock);
                namePinMap[first] = newHardBlock->pin;           
            }
        }
    }    
}

void Input::ReadNets(string filename) {
    // NumNets : 885
    // NumPins : 1873
    // NetDegree : 2
    // p1
    // sb26

    ifstream netfile(filename);
    string title, mark;
    while (netfile >> title) {
        if (title == "NetDegree") {
            Net *newNet = new Net();
            nets.push_back(newNet);

            int degree;
            netfile >> mark >> degree;
            for (int i=0; i<degree; i++) {
                string pinName;
                netfile >> pinName;
                if (namePinMap.find(pinName) == namePinMap.end()) {
                    cout << "Debug: Something wrong with reading pl or hardblock\n";
                }
                newNet->pins.push_back(namePinMap[pinName]);
            }
        }
    }

}

void Input::ReadPl(string filename) {
    // p1	0	0
    ifstream plfile(filename);
    string name;
    int x, y;
    while (plfile >> name >> x >> y) {
        Pin *newPin = new Pin(name, x, y);
        namePinMap[name] = newPin;
    }
}

HardBlock::HardBlock(string n, int w, int h) : name(n), width(w), height(h), x(0), y(0), isRotated(false) {
    pin = new Pin(n, 0, 0);
}

void HardBlock::setValue(int X, int Y, int W, int H_) {
    x = X;
    y = Y;
    isRotated = (width == W && height == H_) ? false : true;
    pin->x = X + W / 2;
    pin->y = Y + H_ / 2;
}

Pin::Pin(string name, int x, int y) : name(name), x(x), y(y) {

}


Floorplan::Floorplan(Input *ip) :input(ip) {
    totalArea = 0;
    for (auto hardblock : input->hardblocks) {
        totalArea += hardblock->height * hardblock->width;
    }
    range = sqrt(totalArea * (1 + input->deadSpaceRatio));
}

void Floorplan::initialConstructNodes() {
    for (auto hardblock : input->hardblocks) {
        Node *newNode = new Node(B, hardblock);
        HBNodes.push_back(newNode);
    }
    int cnt = 0;
    while (cnt < input->hardblocks.size()-1) {
        Node *newNode = new Node(V, nullptr);
        VHNodes.push_back(newNode);
        cnt++;
    }
}

void Floorplan::initialFloorplan() {
    int currentWidth = 0;
    int stackX = 0, stackY = 0;
    int numOP = 0;

    cout << "Debug: Range = " << range << endl;

    for (int i=0; i<input->hardblocks.size(); i++) {
        HardBlock *hardblock = input->hardblocks[i];
        if (currentWidth + hardblock->width > range) {
            stackY++;
            stackX = 1;
            currentWidth = hardblock->width;
            if (stackY >= 2) {
                intialNPE.push_back(H);
                numOP++;
                stackY = 1;
            }
            intialNPE.push_back(i);
        }
        else {
            intialNPE.push_back(i);
            stackX++;
            currentWidth += hardblock->width;
            if (stackX >= 2) {
                intialNPE.push_back(V);
                numOP++;
                stackX = 1;    
            }
        }
    }

    intialNPE.push_back(H);
    numOP++;
    if (numOP != input->hardblocks.size()-1) {
        cout << "Debug: Initial Floorplan goes wrong!\n";
        cout << "Debug: numOP = " << numOP << " " << "hardblock size = " << input->hardblocks.size() << "\n";
    }
    
}

void Floorplan::chainInvert(vector<int> &npe, int pos) {
    while (pos < npe.size() && npe[pos] < B) {
        npe[pos] = (npe[pos] == V) ? H : V;
        pos++;
    }
}

bool Floorplan::checkIfSkewed(vector<int> &npe, int pos) {
    // validate type 3 move
    // only need to check whether the (V,H) violate

    // second pos is (V,H)
    if (pos > 1) {
        if (npe[pos+1] < B && npe[pos-1] != npe[pos+1]) return true;
    }
    // first pos is (V,H)
    else if (pos+2 < npe.size()) {
        if (npe[pos] < B && npe[pos+2] != npe[pos]) return true;
    }

    return false;
}

bool Floorplan::checkIfBalloting(vector<int> &npe, int pos) {
    // check from 0 ~ pos+1 since pos+2 remain the same
    // and we just need to check (V,H) be the second pos case
    // because the other case makes operands > operators
    int opr, opd;
    opr = opd = 0;
    if (npe[pos] >= B) {
        for (int i=0; i<=pos-1; i++) {
            if (npe[i] >= B) opd++;
            else opr++;

            if (opr >= opd) return false;
        }
    }
    opr++;
    if (opr >= opd) return false;
    return true;
}

vector<int> Floorplan::perturbation(vector<int> npe, int type) {
    vector<int> candidate;
    int firstIdx, secondIdx;
    int first, second;

    switch(type) {
    // Operand Swap
    case 1:
        for (int i=0; i<npe.size(); i++) {
            // Get all the hardblocks
            if (npe[i] >= B) candidate.push_back(i);
        }

        firstIdx = rand() % candidate.size();
        do {
            secondIdx = rand() % candidate.size();
        } while (secondIdx == firstIdx);
        
        first = candidate[firstIdx];
        second = candidate[secondIdx];
        swap(npe[first], npe[second]);
    break;

    // Chain invert
    case 2:
        for (int i=1; i<npe.size(); i++) {
            // just get first H or V
            if (npe[i] < B && npe[i-1] >= B) candidate.push_back(i);
        }
        firstIdx = rand() % candidate.size();

        first = candidate[firstIdx];
        chainInvert(npe, first);
    break;

    // Operator/Operand Swap
    case 3:
        int numTry = 0;
        // get the first pos of adjacent pair (e.g. 6H, V3)
        for (int i=0; i<npe.size()-1; i++) {
            // if 6H or V3
            if ((npe[i] >= B && npe[i+1] < B) || (npe[i] < B && npe[i+1] >= B)) candidate.push_back(i);
        }
        
        do {
            firstIdx = rand() % candidate.size();
            first = candidate[firstIdx];
            numTry++;
        } while((!checkIfSkewed(npe, first) || !checkIfBalloting(npe, first)) && numTry < candidate.size());

        if (numTry < candidate.size()) {
            swap(npe[first], npe[first+1]);
        }
        else {
            // cout << "Debug: Not able to do Operator/Operand Swap.\n";
        }

        break;

    }

    // debugBallot(npe);
    return npe;
}

Node* Floorplan::constructTreeViaPostOrder(vector<int>& npe) {
    stack <Node *> stk;
    int idx = 0;

    for (auto e : npe) {
        if (e >= B) {
            stk.push(HBNodes[e]);
        }
        else {
            Node *node = VHNodes[idx];            
            idx++;
            node->kind = e;
            node->right = stk.top();
            stk.pop();
            node->left = stk.top();
            stk.pop(); 
            stk.push(node); 
            stockMeyer(node);
        }
    }
    // cout << Bcnt << " " << VHcnt << endl;

    
    
    Node *root = stk.top();
    stk.pop();
    if (!stk.empty()) {
        cout << "Debug: error occurs at construct tree\n";
    }

    return root;
}

bool Vcmp(Shape const &a, Shape const &b) {
    return a.height > b.height;
}

bool Hcmp(Shape const &a, Shape const &b) {
    return a.width > b.width;
}

void Floorplan::stockMeyer(Node *node) {
    node->possibleShape.clear();

    if (node->kind == V) {
        sort(node->left->possibleShape.begin(), node->left->possibleShape.end(), Vcmp);
        sort(node->right->possibleShape.begin(), node->right->possibleShape.end(), Vcmp);

        int lt, rt;
        lt = rt = 0;

        while (lt < node->left->possibleShape.size() && rt < node->right->possibleShape.size()) {
            int w = node->left->possibleShape[lt].width + node->right->possibleShape[rt].width;
            int h = max(node->left->possibleShape[lt].height, node->right->possibleShape[rt].height);
            Shape sp(w, h, lt, rt);
            node->possibleShape.push_back(sp);

            if (node->left->possibleShape[lt].height > node->right->possibleShape[rt].height) {
                lt++;
            }
            else if (node->left->possibleShape[lt].height < node->right->possibleShape[rt].height) {
                rt++;
            }
            else {
                lt++;
                rt++;
            }
        }
    }
    else {
        sort(node->left->possibleShape.begin(), node->left->possibleShape.end(), Hcmp);
        sort(node->right->possibleShape.begin(), node->right->possibleShape.end(), Hcmp);

        int lt, rt;
        lt = rt = 0;

        while (lt < node->left->possibleShape.size() && rt < node->right->possibleShape.size()) {
            int w = max(node->left->possibleShape[lt].width, node->right->possibleShape[rt].width);
            int h = node->left->possibleShape[lt].height + node->right->possibleShape[rt].height;
            Shape sp(w, h, lt, rt);
            node->possibleShape.push_back(sp);

            if (node->left->possibleShape[lt].width > node->right->possibleShape[rt].width) {
                lt++;
            }
            else if (node->left->possibleShape[lt].width < node->right->possibleShape[rt].width) {
                rt++;
            }
            else {
                lt++;
                rt++;
            }
        }
    }
}

int Floorplan::calculateHPWL(Net *net) {
    int minx, miny;
    int maxx, maxy;
    minx = miny = INT_MAX;
    maxx = maxy = INT_MIN;
    for (auto p : net->pins) {
        minx = min(minx, p->x);
        miny = min(miny, p->y);
        maxx = max(maxx, p->x);
        maxy = max(maxy, p->y);
    }
    return ((maxx - minx) + (maxy - miny));
}

int Floorplan::calculateWireLength() {
    int wirelength = 0;
    for (auto net : input->nets) {
        wirelength += calculateHPWL(net);
    }
    return wirelength;
}

int Floorplan::calculateCostFunc(vector<int>& npe) {
    Node *root = constructTreeViaPostOrder(npe);
    // cout << "Test: construct finish\n";
    int minArea = INT_MAX;
    int curArea, whichOne;
    int weight = 10;
    curArea = whichOne = 0;
    
    for (int i=0; i<root->possibleShape.size(); i++) {
        Shape sp = root->possibleShape[i];
        if (sp.width > range && sp.height > range) {
            curArea = (sp.width - range) * (sp.height - range);
        }
        else if (sp.width > range) {
            curArea = (sp.width - range) * range;
        }
        else if (sp.height > range) {
            curArea = range * (sp.height - range);
        }
        else { 
            // the shape is in the fixed area
            curArea = 0;
            cout << "Debug: Floorplan is in the fixed area\n";
        }

        if (minArea > curArea) {
            minArea = curArea;
            whichOne = i;
        }
    }

    setHardBlockCoord(root, 0, 0, whichOne);
    int wirelength = calculateWireLength();
    wirelength = 0;
    return minArea * weight + wirelength;
}

void Floorplan::setHardBlockCoord(Node *node, int coordX, int coordY, int whichOne) {
    if (node->kind >= B) {
        node->hardblock->setValue(coordX, coordY, node->possibleShape[whichOne].width, node->possibleShape[whichOne].height);
    }
    else if (node->kind == V) {
        setHardBlockCoord(node->left, coordX, coordY, node->possibleShape[whichOne].leftShape);
        int _x = node->left->possibleShape[node->possibleShape[whichOne].leftShape].width;
        setHardBlockCoord(node->right, coordX + _x, coordY, node->possibleShape[whichOne].rightShape);
    }
    else if (node->kind == H) {
        setHardBlockCoord(node->left, coordX, coordY, node->possibleShape[whichOne].leftShape);
        int _y = node->left->possibleShape[node->possibleShape[whichOne].leftShape].height;
        setHardBlockCoord(node->right, coordX, coordY + _y, node->possibleShape[whichOne].rightShape);
    }
}

vector<int> Floorplan::SA_Floorplanning(double e, double r, int k) {
    int MT, uphill, reject, N, bestCost;
    vector<int> Best, E;
    double T = 1000;

    MT = uphill = reject = 0;
    N = k * input->hardblocks.size();
    E = this->intialNPE;
    Best = E;
    bestCost = calculateCostFunc(Best);

    do {
        MT = uphill = reject = 0;
        do {
            int M = rand() % 3;
            M += 1;
            // M = 3;
            // cout << "Test: Move = " << M << endl;
            vector<int> NE = perturbation(E, M);

            MT++;
            int deltaCost = calculateCostFunc(NE) - calculateCostFunc(E);
            // cout << "Test: done\n";

            double rd = (double)rand();
            if ((deltaCost <= 0) || rd < exp(-1*deltaCost / T)) {
                if (deltaCost > 0) {
                    uphill++;
                }
                E = NE;

                int ECost = calculateCostFunc(E);
                if (ECost < bestCost) {
                    Best = E;
                    bestCost = ECost;
                }
            }
            else {
                reject++;
            }
            // cout << "Test: Run\n";

        } while (uphill <= N && MT <= 2*N);
        T = r*T;
    } while ((reject/MT) <= 0.95 && T >= e);

    return Best;
}

void Floorplan::Algorithm() {
    srand((int)time(NULL));
    initialFloorplan();
    cout << "Debug: Finish initial floorplan\n";
    initialConstructNodes();
    cout << "Debug: Finish initial construct nodes\n";
    // calculateCostFunc(this->npe);

    SA_Floorplanning(0.1, 0.9, 10);
    // test();
}

// output
void Floorplan::outputResult(string filename) {
    ofstream outfile(filename);
    int wirelength = calculateWireLength();
    outfile << "Wirelength " << wirelength << "\n";
    outfile << "Blocks\n";
    for (auto hardblock : input->hardblocks) {
        outfile << hardblock->name << " " << hardblock->x << " " << hardblock->y << " " << hardblock->isRotated << "\n";
    }
}

void Floorplan::test() {
    for (auto e : intialNPE) {
        cout << e << " ";
    }
    cout << "\n";
}

void Floorplan::debugBallot(vector<int> _n) {
    int i, j;
    i = j = 0;
    for (auto e : _n) {
        if (e >= B) i++;
        else j++;
        
        if (j >= i) {
            cout << "Test: Critical error\n";
            return;
        } 
    }
}

Node::Node(int k, HardBlock *hb) :left(nullptr), right(nullptr), kind(k), hardblock(hb) {
    if (hb != nullptr) {
        Shape sp1(hb->width, hb->height, 0, 0);
        Shape sp2(hb->height, hb->width, 1, 1);
        possibleShape.push_back(sp1);
        possibleShape.push_back(sp2);
    }
}

Shape::Shape(int w, int h, int l, int r) :width(w), height(h), leftShape(l), rightShape(r) {

}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        cout << "missing some parameters\n";
        return 0;
    }

    clock_t IOtime, Exectime;
    clock_t loadTime1, loadTime2;
    clock_t ecTime1, ecTime2;
    clock_t outputTime1, outputTime2; 

    Input *_input = new Input();
    _input->ReadAllFile(argv);
    cout << "Debug: Finish loading three files\n";

    Floorplan FP(_input);
    FP.Algorithm();
    cout << "Debug: Finish Floorplanning\n";

    FP.outputResult(argv[4]);
    cout << "Debug: Finish writing the result\n";

    return 0;
}
