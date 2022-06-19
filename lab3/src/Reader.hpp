#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

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
    void setValue(int X, int Y, int W, int H);
};

class Net {
public:
    vector<Pin *> pins;
};