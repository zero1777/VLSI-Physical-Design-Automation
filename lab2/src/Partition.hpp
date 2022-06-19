#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <stack>
#include "LCN.hpp"

using namespace std;


class Group {
public:
    int Pmax;
    int groupSize;
    int numOfCell;
    unordered_map<int, Node *> bucketList;
    Group();
    void InsertNode(Cell *cell);
    void RemoveNode(Cell *cell);
    void UpdateNode(Cell *cell);
};

class FM_Algorithm {
public:
    int Pmax;
    Input *input;
    vector<Group *> vgroup;
    queue<Cell *> cellRecord;
    stack<Cell *> cellStack;
    int bestResult;
    int minCutSize;
    int testSeed;

    FM_Algorithm(Input *ip);
    
    // Initialize part
    void Init();
    void RandomPartition();
    void CalculateInitNetAB();
    void CalculateInitGain();
    void CalculatePmax();
    void BuildInitBucketList();

    // Loop part
    int Loop(bool firstTime);
    void CalculateNetAB();
    void CalculateGain();
    void BuildBucketList();
    void FreeCellLock();
    int UpdateGain(Cell *baseCell);
    Cell* GetBaseCell(int cannotMove);
    Cell* GetBaseCellFromGroup(int group);
    bool CheckIfBalanced(Cell *baseCell);
    void ForwardToMaxPartialSum(int step);
    void UpdateGroupSize(int from, int to, Cell *cell);

    // Run FM algorithm
    void Run();

    // Verify the result
    int CalculateCutSize();

    // Output the answer
    void OutputResult(string outputFilename);
    int GetFinalNumOfCell(int group);

    // Test
    void OutputTest(string outputFilename);
    
};