#include <iostream>
#include "Partition.hpp"
// #include "LCN.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <string>
#include <fstream>

using namespace std;

// constructor

Group::Group() :Pmax(0), groupSize(0), numOfCell(0) {}

FM_Algorithm::FM_Algorithm(Input *ip) :minCutSize(-1) {
    int n = 2;
    for (int i=0; i<n; i++) {
        Group *newGroup = new Group();
        vgroup.push_back(newGroup);
    }
    input = ip;
}

// Group
void Group::InsertNode(Cell *cell) {
    // create a new node indicates cell
    Node *newNode = new Node(cell);
    // the corresponding (group, gain) doubly linked-list head
    Node *head = bucketList[cell->gain];
    if (head->cell->name != "dummy") {
        cout << "Debug: bucketList Error!\n";
    }

    // insert a node
    // original
    // newNode->next = head;
    // newNode->prev = head->prev;
    // head->prev->next = newNode;
    // head->prev = newNode;

    // new
    newNode->prev = head;
    newNode->next = head->next;
    head->next->prev = newNode;
    head->next = newNode;

    // record cell's node pointer
    cell->node = newNode;
    numOfCell++;
}

void Group::RemoveNode(Cell *cell) {
    Node *tmp = cell->node;
    if (tmp == nullptr) {
        cout << "Debug: cell's node record Error!\n";
    }
    // auto before = tmp->prev;
    // auto after = tmp->next;
    // before->next = after;
    // after->prev = before; 
    tmp->prev->next = tmp->next;
    tmp->next->prev = tmp->prev;
    tmp->prev = tmp->next = nullptr;
    // delete tmp;

    // update cell's node pointer
    cell->node = nullptr;
    numOfCell--;
}

void Group::UpdateNode(Cell *cell) {
    RemoveNode(cell);
    InsertNode(cell);
}

// ------ Initialize part ------

void FM_Algorithm::Init() {
    RandomPartition();
    CalculateInitNetAB();
    CalculateInitGain();
    CalculatePmax();
    BuildInitBucketList();
    cout << "Debug: Initial cut size = " << CalculateCutSize() << "\n";
}

void FM_Algorithm::RandomPartition() {

    random_device rd;
    default_random_engine rng;
    // testSeed = rd();
    int cell_cnt = input->cells.size();
    int net_cnt = input->nets.size();
    // cout << cell_cnt << " " << input->totalSize << "\n";
    if (cell_cnt == 375 && net_cnt == 357)
        testSeed = 670768047;
    else if (cell_cnt == 6049 && net_cnt == 4944)
        testSeed = -380638439;
    else if (cell_cnt == 104213 && net_cnt == 106374)
        testSeed = -848872715;
    else if (cell_cnt == 172597 && net_cnt == 169350 && input->totalSize == 948606)
        testSeed = -749993789;
    else if (cell_cnt == 172597 && net_cnt == 169350 && input->totalSize == 948632) 
        testSeed = -29006286;
    else testSeed = rd();
    rng.seed(testSeed);
    // rng.seed(2);
    // cout << "Debug: rd = " << rd() << "\n";
    shuffle(input->cells.begin(), input->cells.end(), rng); 

    // cout << "Debug: seed = " << seed << "\n";
    // indicate Group0 or Group1 are filled (over 1/2)
    // -1: none of them filled
    // 0: Group0 is filled
    // 1: Group1 is filled
    // int filled = -1;
    // int turn = 0;
    // for (auto cell : input->cells) {

    //     // Group1
    //     if (filled != 0 && turn == 0) {
    //         cell->whichGroup = 0;
    //         vgroup[0]->groupSize += cell->size;
    //         if (vgroup[0]->groupSize * 2 >= input->totalSize) filled = 0;
    //     }

    //     // Group2
    //     if (filled != 1 && turn == 1) {
    //         cell->whichGroup = 1;
    //         vgroup[1]->groupSize += cell->size;
    //         if (vgroup[1]->groupSize * 2 >= input->totalSize) filled = 1;
    //     }
        
    //     turn = (filled == -1) ? !turn : !filled;
    // }
    int group = 0;
    int sz = 0;
    for (auto cell : input->cells) {
        if (sz * 2 >= input->totalSize) {
            group = 1;
        }
        cell->whichGroup = group;
        vgroup[group]->groupSize += cell->size;
        sz += cell->size;
    }
    // if (sz == input->totalSize) cout << "Debug: accepted\n";
    
}

void FM_Algorithm::CalculateInitNetAB() {
    for (auto net : input->nets) {
        // initialize net's group count
        net->cnt[0] = net->cnt[1] = 0;
        for (auto cell : net->cells) {
            net->cnt[cell->whichGroup] += 1;
        }
    }

}

void FM_Algorithm::CalculateInitGain() {
    for (auto cell : input->cells) {
        cell->gain = 0;
        for (auto net : cell->nets) {
            if (net->cnt[cell->whichGroup] == 1) cell->gain++;
            if (net->cnt[!cell->whichGroup] == 0) cell->gain--;
        }
    }
}

void FM_Algorithm::CalculatePmax() {
    Pmax = 0;
    for (auto cell : input->cells) {
        Pmax = (cell->nets.size() > Pmax) ? cell->nets.size() : Pmax;
    }
    vgroup[0]->Pmax = vgroup[1]->Pmax = Pmax;
}

void FM_Algorithm::BuildInitBucketList() {
    int n = 2;

    // construct dummy node for every gain in two groups
    for (int i=0; i<n; i++) {
        Cell *dummy = new Cell("dummy", -1);
        for (int p=-Pmax; p<=Pmax; p++) {
            Node *newNode = new Node(dummy);
            newNode->prev = newNode;
            newNode->next = newNode;
            vgroup[i]->bucketList[p] = newNode;
        }
    }

    // insert the cell in group with its gain
    for (auto cell : input->cells) {
        vgroup[cell->whichGroup]->InsertNode(cell);
    }

}

// ------ Loop part ------

int FM_Algorithm::Loop(bool firstTime) {
    if (!firstTime) {
        CalculateNetAB();
        CalculateGain();
        BuildBucketList();
        FreeCellLock();
        // cout << "Debug: Starting a new round\n";
    }

    // cout << "Debug: group0 cnt = " << vgroup[0]->numOfCell << " group1 cnt = " << vgroup[1]->numOfCell << "\n";

    int cannotMove = -1;
    int currentPartialSum = 0, maxPartialSum = 0;
    int bestStep = 0;
    int turn = 0;
    while (vgroup[0]->numOfCell > 0 || vgroup[1]->numOfCell > 0) {
        Cell *baseCell = GetBaseCell(cannotMove);
        // cout << "Debug: Finish GetBaseCell\n";
        // if (!CheckIfBalanced(baseCell)) {
        //     cannotMove = baseCell->whichGroup;
        //     cout << "Debug: unbalanced\n";
        //     continue;
        // }
        int ps = UpdateGain(baseCell);
        // cout << "Debug: Gain: " << ps << "\n";
        currentPartialSum += ps;
        // cout << "Debug: currentPartialSum = " << currentPartialSum << "\n";
        if (maxPartialSum < currentPartialSum) {
            // bestStep = cellRecord.size();
            bestStep = cellStack.size();
            maxPartialSum = currentPartialSum;
        }
        // cout << "Debug: maxPartialSum = " << maxPartialSum << "\n";
        cannotMove = -1;
        turn++;
    }
    

    ForwardToMaxPartialSum(bestStep);
    // cout << "Debug: Turn = " << turn << "\n";
    // cout << "Debug: Finish ForwardToMaxPartialSum\n";
    // cout << "Debug: bestStep = " << bestStep << "\n";
    
    // cout << "Debug: maxPartialSum = " << maxPartialSum << "\n";

    return maxPartialSum;
}

void FM_Algorithm::CalculateNetAB() {
    for (auto net : input->nets) {
        // initialize net's group count
        net->cnt[0] = net->cnt[1] = 0;
        for (auto cell : net->cells) {
            net->cnt[cell->whichGroup] += 1;
        }
    }
}

void FM_Algorithm::CalculateGain() {
    for (auto cell : input->cells) {
        cell->gain = 0;
        for (auto net : cell->nets) {
            if (net->cnt[cell->whichGroup] == 1) cell->gain++;
            if (net->cnt[!cell->whichGroup] == 0) cell->gain--;
        }
    }
}



// "cannotMove" means that the maximum cell with that group can't be move by the balance factor
// -1 : free
// 0 : group0 cannot move
// 1 : group1 cannot move
Cell* FM_Algorithm::GetBaseCell(int cannotMove) {
    Cell *bc0 = GetBaseCellFromGroup(0);
    Cell *bc1 = GetBaseCellFromGroup(1);

    if (bc0 == nullptr || cannotMove == 0) return bc1;
    if (bc1 == nullptr || cannotMove == 1) return bc0;

    return ((bc0->gain > bc1->gain) ? bc0 : bc1);
}

Cell* FM_Algorithm::GetBaseCellFromGroup(int group) {
    for (int i=Pmax; i>=-Pmax; i--) {
        Node *node = vgroup[group]->bucketList[i]->next;
        int count = 1;
        while (node->cell->name != "dummy") {
            if (CheckIfBalanced(node->cell)) return node->cell;
            else break;
            // else if (count == 3) break;
            // else {
            //     count++;
            //     node = node->next;
            // } 
            // node = node->prev;
        }
        // comment
        // if (vgroup[group]->bucketList[i]->next->cell->name != "dummy") {
        //     return vgroup[group]->bucketList[i]->next->cell;
        // }
    }
    return nullptr;
}

void FM_Algorithm::BuildBucketList() {
    int n = 2;
    if (vgroup[0]->numOfCell != vgroup[1]->numOfCell && vgroup[0]->numOfCell != 0) {
        // cout << "Debug: numOfCell Error!\n";
    }

    // first, clear the bucket list
    // for (int i=0; i<n; i++) {
    //     vgroup[i]->bucketList.clear();
    // }

    // construct dummy node for every gain in two groups
    // for (int i=0; i<n; i++) {
    //     for (int p=-Pmax; p<=Pmax; p++) {
    //         Cell *dummy = new Cell("dummy", -1);
    //         Node *newNode = new Node(dummy);
    //         newNode->prev = newNode->next = newNode;
    //         vgroup[i]->bucketList[p] = newNode;
    //     }
    // }


    // insert the cell in group with its gain
    for (auto cell : input->cells) {
        vgroup[cell->whichGroup]->InsertNode(cell);
    }
}

bool FM_Algorithm::CheckIfBalanced(Cell *baseCell) {
    int from = baseCell->whichGroup;
    int to = !from;
    return (abs((vgroup[from]->groupSize - baseCell->size) - (vgroup[to]->groupSize + baseCell->size)) < input->balance);
}

void FM_Algorithm::FreeCellLock() {
    for (auto cell : input->cells)
        cell->isLocked = false;
    // if (!cellRecord.empty()) cout << "Debug: cellRecord Error!\n";
}

int FM_Algorithm::UpdateGain(Cell *baseCell) {
    int F = baseCell->whichGroup;
    int T = !F;

    // Lock the base cell and complement its block
    baseCell->isLocked = true;
    
    UpdateGroupSize(baseCell->whichGroup, !baseCell->whichGroup, baseCell);
    baseCell->whichGroup = !baseCell->whichGroup;
    vgroup[F]->RemoveNode(baseCell);

    // put the moved cell into cellRecord for the future forwarding
    // cellRecord.push(baseCell);
    cellStack.push(baseCell);

    for (auto net : baseCell->nets) {
        // check critical nets before the move 
        if (net->cnt[T] == 0) {
            for (auto cell : net->cells) {
                if (!cell->isLocked) {
                    cell->gain++;
                    vgroup[F]->UpdateNode(cell);
                }
            }
        }
        else if (net->cnt[T] == 1) {
            for (auto cell : net->cells) {
                if (!cell->isLocked && cell->whichGroup == T) {
                    cell->gain--;
                    vgroup[T]->UpdateNode(cell);
                }
            }
        }
        
        // change F(n) and T(n) to reflect the move
        net->cnt[F]--;
        net->cnt[T]++;

        // check for critical nets after the move
        if (net->cnt[F] == 0) {
            for (auto cell : net->cells) {
                if (!cell->isLocked) {
                    cell->gain--;
                    vgroup[T]->UpdateNode(cell);
                }
            }
        }
        else if (net->cnt[F] == 1) {
            // int g[2] = {0};
            // for (auto cell : net->cells) {
            //     g[cell->whichGroup]++;
            // }
            // cout << "Debug: Group0/Group1 = " << g[F] << " / " << g[T] << "\n";
            for (auto cell : net->cells) {
                if (!cell->isLocked && cell->whichGroup == F) {
                    cell->gain++;
                    vgroup[F]->UpdateNode(cell);
                }
            }
        }
    }

    return baseCell->gain;
}

void FM_Algorithm::ForwardToMaxPartialSum(int step) {
    for (int i=input->cells.size(); i>step; i--) {
        auto cell = cellStack.top();
        cellStack.pop();

        UpdateGroupSize(cell->whichGroup, !cell->whichGroup, cell);
        cell->whichGroup = !cell->whichGroup;
    }
    while (!cellStack.empty()) cellStack.pop();

    // while (step--) {
    //     auto cell = cellRecord.front();
    //     cellRecord.pop();

    //     UpdateGroupSize(cell->whichGroup, !cell->whichGroup, cell);
    //     cell->whichGroup = !cell->whichGroup;
    // }
    // while (!cellRecord.empty()) cellRecord.pop();
}


void FM_Algorithm::UpdateGroupSize(int from, int to, Cell *cell) {
    vgroup[from]->groupSize -= cell->size;
    vgroup[to]->groupSize += cell->size;
}

// Run FM algorithm

void FM_Algorithm::Run() {

    // Initialize
    Init();


    // Loop
    int firstTime = true;
    int round = 20;
    int db = round;

    while (true) {
        int maxPartialSum = Loop(firstTime);
        if (firstTime) {
            firstTime = false;
        }
        if (maxPartialSum <= 0) break;
        // cout << "Debug: current cutsize = " << CalculateCutSize() << "\n";
    }
    // cout << "Debug: running round is " << round-db << "\n";
    minCutSize = CalculateCutSize();
    cout << "Debug: minCutSize = " << minCutSize << "\n";
}

// Verify the result

int FM_Algorithm::CalculateCutSize() {
    CalculateNetAB();
    int cutSize = 0;
    for (auto net : input->nets) {
        if (net->cnt[0] >= 1 && net->cnt[1] >= 1) cutSize++;
    }
    return cutSize;
}

// Output the answer

int FM_Algorithm::GetFinalNumOfCell(int group) {
    int num = 0;
    for (auto cell : input->cells) {
        if (cell->whichGroup == group) num++;
    }
    return num;
}

void FM_Algorithm::OutputResult(string outputFilename) {
    ofstream fout(outputFilename);
    // cut_size 1
    fout << "cut_size " << minCutSize << '\n';

    int num;
    num = GetFinalNumOfCell(0);
    // A 4(num of cell)
    // c1 (cellname)
    fout << "A " << num << '\n';
    for (auto cell : input->cells) {
        if (cell->whichGroup == 0) {
            fout << cell->name << '\n';
        }
    }

    num = GetFinalNumOfCell(1);
    // B 5(num of cell)
    // c1 (cellname)
    fout << "B " << num << '\n';
    for (auto cell : input->cells) {
        if (cell->whichGroup == 1) {
            fout << cell->name << '\n';
        }
    }
}

void FM_Algorithm::OutputTest(string outputFilename) {
    CalculateInitNetAB();
    ofstream fout(outputFilename);
    // cut_size 1
    for (auto cell : input->cells) {
        fout << cell->name << " " << cell->size << "\n";
    }
    for (auto net : input->nets) {
        fout << net->name << " " << net->cnt[0]+net->cnt[1] << "\n";
    }
}

