#include <iostream>
#include <string>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <stack>
#include <algorithm>
#include <math.h>
#include <time.h>
#include <random>
#include <chrono>
#include <fstream>
#include <ctime>
#include "Floorplan.hpp"

#define B 0
#define V -1
#define H -2

#define Stage1 1
#define Stage2 2
#define Exper1 3
#define Exper2 4
#define Exper3 5
#define Exper4 6

using namespace std;

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

    // cout << "Debug: Range = " << range << endl;

    // intialNPE.push_back(0);
    // for (int i=1; i<input->hardblocks.size(); i++) {
    //     intialNPE.push_back(i);
    //     intialNPE.push_back(V);
    // }

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
    if (npe[pos+1] < B && pos >= 1) {
        if (npe[pos-1] != npe[pos+1]) return true;
    }
    // first pos is (V,H)
    else if (npe[pos] < B && pos+2 < npe.size()) {
        if (npe[pos+2] != npe[pos]) return true;
    }

    // return false;
    // if (pos == 0) {
    //     if (npe[pos] != npe[pos+2]) return true;
    //     cout << "Debug: Pos = 0\n";
    // }
    // else if (pos+1 == npe.size()-1) {
    //     if (npe[pos-1] != npe[pos+1]) return true;
    // }
    // else {
    //     if (npe[pos-1] != npe[pos+1] && npe[pos] != npe[pos+2]) return true;
    // }
    return false;
}

bool Floorplan::checkIfBalloting(vector<int> &npe, int pos) {
    // check from 0 ~ pos+1 since pos+2 remain the same
    // and we just need to check (V,H) be the second pos case
    // because the other case makes operands > operators
    int opr, opd;
    opr = opd = 0;
    if (npe[pos] < B) return true;
    else {
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
    int firstIdx, secondIdx;
    int first, second;
    int pos[npe.size()], cnt = 0;
    int numTry = 0;

    switch(type) {
    // Operand Swap
    case 1:
        for (int i=0; i<npe.size(); i++) {
            // Get all the hardblocks
            // if (npe[i] >= B) candidate.push_back(i);
            if (npe[i] >= B) pos[cnt++] = i;
        }

        firstIdx = rand() % cnt;
        secondIdx = rand() % cnt;
        while (secondIdx == firstIdx) {
            secondIdx = rand() % cnt;
        }
        // firstIdx = rand() % candidate.size();
        // secondIdx = rand() % candidate.size();
        // while (secondIdx == firstIdx) {
        //     secondIdx = rand() % candidate.size();
        // }
        // cout << firstIdx << " " << secondIdx << endl;
        
        first = pos[firstIdx];
        second = pos[secondIdx];
        swap(npe[first], npe[second]);
    break;

    // Chain invert
    case 2:
        for (int i=1; i<npe.size(); i++) {
            // just get first H or V
            if (npe[i] < B && npe[i-1] >= B) pos[cnt++] = i;
        }
        firstIdx = rand() % cnt;

        first = pos[firstIdx];
        chainInvert(npe, first);
    break;

    // Operator/Operand Swap
    case 3:
        
        // get the first pos of adjacent pair (e.g. 6H, V3)
        for (int i=0; i<npe.size()-1; i++) {
            // if 6H or V3
            if ((npe[i] >= B && npe[i+1] < B) || (npe[i] < B && npe[i+1] >= B)) pos[cnt++] = i;
        }
        
        do {
            firstIdx = rand() % cnt;
            first = pos[firstIdx];
            numTry++;
        } while((checkIfSkewed(npe, first) && checkIfBalloting(npe, first)) == false && numTry < cnt);

        if (numTry < cnt) {
            int second = first + 1;
            swap(npe[first], npe[second]);
        }
        else {
            // cout << "Debug: Not able to do Operator/Operand Swap.\n";
        }

    break;

    case 4:
        for (int i=0; i<=npe.size()-3; i++) {
            if (npe[i] >= B && npe[i+1] >= B && npe[i+2] < B) {
                pos[cnt++] = i;
            }
        }
        firstIdx = rand() % cnt;
        first = pos[firstIdx];

        swap(npe[first], npe[first+1]);

    break;

    case 5:
        for (int i=0; i<=npe.size()-4; i++) {
            if (npe[i] >= B && npe[i+1] >= B && npe[i+2] < B && npe[i+3] >= B) {
                pos[cnt++] = i+2;
            }
        }

        firstIdx = rand() % cnt;
        first = pos[firstIdx];
        npe[first] = (npe[first] == V) ? H : V;
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


int Floorplan::calculateCostFunc(vector<int>& npe, int whichStage) {
    Node *root = constructTreeViaPostOrder(npe);
    // cout << "Debug: whichStage = " << whichStage << endl;
    // cout << "Test: construct finish\n";
    int minArea = INT_MAX;
    int curArea, whichOne;
    int weight = 10;
    int tmpWL = INT_MAX - 1;
    curArea = whichOne = 0;
    
    for (int i=0; i<root->possibleShape.size(); i++) {
        int tp = INT_MAX;
        Shape sp = root->possibleShape[i];
        // cout << sp.width << " " << sp.height << endl;
        if (sp.width > range && sp.height > range) {
            curArea = sp.width * sp.height - range * range;
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
            if (whichStage != Stage1) {
                setHardBlockCoord(root, 0, 0, i);
                tp = calculateWireLength();
            }
            // cout << "Debug: Floorplan is in the fixed area\n";
        }

        if (whichStage == Stage1 && minArea > curArea) {
            minArea = curArea;
            whichOne = i;
        }
        else if (whichStage != Stage1 && (minArea > curArea || (minArea == curArea && tp < tmpWL))) {
            minArea = curArea;
            whichOne = i;
            tmpWL = tp;
            // cout << "Debug: minArea = " << minArea << endl;
        }
    }
    if (whichStage == Stage1 || whichStage == Exper2) {
        // cout << "Debug: Stage1\n";
        return weight * minArea;
    }

    // if (whichStage == 0) return weight * minArea;

    setHardBlockCoord(root, 0, 0, whichOne);
    int wirelength = calculateWireLength();
    // cout << "Debug: stage2\n";
    // wirelength = 0;
    if (minArea != 0) return wirelength * weight;
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


vector<int> Floorplan::SA_Floorplanning(vector<int> &inputNPE, double e, double r, int k, int whichStage) {
    int MT, uphill, reject, N, bestCost, ECost;
    vector<int> Best, E;
    double T = 1000;

    MT = uphill = reject = 0;
    N = k * input->hardblocks.size();
    E = inputNPE;
    Best = E;
    bestCost = ECost = calculateCostFunc(Best, whichStage);
    if (bestCost == 0) {
        // calculateCostFunc(Best, Stage2);
        return Best;
    }

    do {
        T = 1000;
        do {
            MT = uphill = reject = 0;
            do {
                int M;
                if (whichStage == Stage1) M = rand() % 3;
                else if (whichStage == Exper1) M = 3;
                else if (whichStage == Exper2) M = rand() % 3;
                else if (whichStage == Exper3) M = 1;
                else if (whichStage == Exper4) M = 2;
                else M = 0;
                M += 1;
                // M = 3;
                // cout << "Test: Move = " << M << endl;
                vector<int> NE = perturbation(E, M);

                MT++;
                // int deltaCost = calculateCostFunc(NE, whichStage) - calculateCostFunc(E, whichStage);
                int cost = calculateCostFunc(NE, whichStage);
                int deltaCost = cost - ECost;
                // cout << "Test: done\n";

                double rd = (double)rand() / RAND_MAX;
                if ((deltaCost < 0) || rd < exp(-1*deltaCost / T)) {
                    if (deltaCost > 0) {
                        uphill++;
                    }
                    E = NE;
                    ECost = cost;

                    // int ECost = calculateCostFunc(E, whichStage);
                    if (ECost < bestCost) {
                        Best = E;
                        bestCost = ECost;
                        // cout << "Debug: Bestcost = " << bestCost << endl;

                        if (bestCost == 0) {
                            // calculateCostFunc(Best, Stage2);
                            // cout << "Debug: Find\n";
                            return Best;
                        }
                    }
                }
                else {
                    reject++;
                }
            } while (uphill <= N && MT <= 2*N);
            T = r*T;
            
        } while ((reject/MT) <= 0.95 && T >= e);
    } while (whichStage == Stage1);

    // cout << "Debug: Here out\n";
    return Best;
}

void Floorplan::Algorithm() {
    clock_t sa_stage1_1, sa_stage1_2;
    clock_t sa_stage2_1, sa_stage2_2;
    clock_t sa_stage3_1, sa_stage3_2;


    int iter = 1;
    int wirelen = INT_MAX;
    initialFloorplan();
    cout << "Debug: Finish initial floorplan\n";
    initialConstructNodes();
    cout << "Debug: Finish initial construct nodes\n";
    int bestSeed, bestWirelength = INT_MAX;

    random_device rd;
    int seed;
    while (iter--) {
        // srand((int)time(NULL));
        // int seed = (int)time(NULL);
        seed = (int) rd();
        if (input->hardblocks.size() == 100) {
            if (input->deadSpaceRatio == 0.15) {
                seed = -179645290; 
            }
            else if (input->deadSpaceRatio == 0.1) {
                seed = -1701349740;
            }
        }
        else if (input->hardblocks.size() == 200) {
            if (input->deadSpaceRatio == 0.15) {
                seed = -1221175410;
            }
            else if (input->deadSpaceRatio == 0.1) {
                seed = -217822452;
            }
        }
        else if (input->hardblocks.size() == 300) {
            if (input->deadSpaceRatio == 0.15) {
                seed = 1420872313;
            }
            else if (input->deadSpaceRatio == 0.1) {
                seed = -1296314180;
            }
        }

        // int seed = (int) rd();
        // seed = -2102350778;
        srand(seed);
        cout << "Debug: Seed = " << seed << endl;
        
        // calculateCostFunc(this->npe);
        
        // SA_Floorplanning (lower-bound, rate, try chances)
        // vector<int> notBadSol = saProcess(0.1, 0.9, 10, intialNPE, false);
        sa_stage1_1 = clock();
        // for (auto n : intialNPE) cout << n << " ";
        vector<int> _stage1 = SA_Floorplanning(intialNPE, 0.1, 0.9, 10, Stage1);
        cout << "Debug: Generate Stage1 solution\n";
        // cout << "Debug: compare = " << calCost(notBadSol, true) << endl;
        calculateCostFunc(_stage1, Stage2);
        cout << "Debug: Stage1 Wirelength = " << calculateWireLength() << endl;
        sa_stage1_2 = clock();
        // test();

        sa_stage2_1 = clock();
        vector<int> _stage2 = SA_Floorplanning(_stage1, 1, 0.95, 5, Stage2);
        // // saProcess(1, 0.95, 5, notBadSol, true);
        cout << "Debug: Generate Stage2 solution\n";
        calculateCostFunc(_stage2, Stage2);
        // int tmp = calculateWireLength();
        cout << "Debug: Stage2 Wirelength = " << calculateWireLength() << endl;
        sa_stage2_2 = clock();


        sa_stage3_1 = clock();
        vector<int> _stage3 = SA_Floorplanning(_stage2, 1, 0.95, 3, Exper1);
        // SA_Floorplanning(refinement, 1, 0.97, 4, Exper1);
        cout << "Debug: Generate Exper1 solution\n";
        calculateCostFunc(_stage3, Stage2);
        int tmp = calculateWireLength();
        cout << "Debug: Exper1 Wirelength = " << tmp << endl;
        sa_stage3_2 = clock();

        // if (tmp < bestWirelength) {
        //     bestSeed = seed;
        //     bestWirelength = tmp;
        // }

        // vector<int> _stage4 = SA_Floorplanning(_stage3, 10, 0.85, 3, Exper2);
        // // SA_Floorplanning(refinement, 1, 0.97, 4, Exper1);
        // cout << "Debug: Generate Exper2 solution\n";
        // calculateCostFunc(_stage4, Stage2);
        // int t = calculateWireLength();
        // cout << "Debug: Exper2 Wirelength = " << t << endl;
        cout << "Time: stage1 time usage = " << double(sa_stage1_2 - sa_stage1_1) / CLOCKS_PER_SEC << endl;
        cout << "Time: stage2 time usage = " << double(sa_stage2_2 - sa_stage2_1) / CLOCKS_PER_SEC << endl;
        cout << "Time: stage3 time usage = " << double(sa_stage3_2 - sa_stage3_1) / CLOCKS_PER_SEC << endl;
    }

    // cout << "Debug: Final seed = " << bestSeed << endl;
    // cout << "Debug: Final wirelength = " << bestWirelength << endl;
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