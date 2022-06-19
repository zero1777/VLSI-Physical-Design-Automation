#include "Reader.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

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

void HardBlock::setValue(int X, int Y, int W, int H) {
    x = X;
    y = Y;
    isRotated = (width == W && height == H) ? false : true;
    pin->x = X + W / 2;
    pin->y = Y + H / 2;
}

Pin::Pin(string name, int x, int y) : name(name), x(x), y(y) {

}
