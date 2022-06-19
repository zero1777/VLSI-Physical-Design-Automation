#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class Cell;
class Net;
class Node;

class Input {
public:
    int totalSize;
    double balance;
    vector<Net *> nets;
    vector<Cell *> cells;
    unordered_map<string, Cell *> handles;

    Input();
    void LoadCellAndNet(char *argv[]);
};

class Cell {
public:
    string name;
    int gain, size, whichGroup;
    bool isLocked;
    vector<Net *> nets;
    Node *node;
    Cell(string name, int size);
};

class Net {
public: 
    string name;
    vector<Cell *> cells;
    int cnt[2]; // group count
    Net(string name);
};

class Node {
public:
    Node *next, *prev;
    Cell *cell;
    Node(Cell *c);
};