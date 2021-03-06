#include <iostream>
#include <fstream>
#include <string>
#include "LCN.hpp"

using namespace std;

// Constructor
Input::Input() :totalSize(0) {}

Cell::Cell(string name, int size) :name(name), size(size), isLocked(false), gain(0), whichGroup(0) {} 

Net::Net(string name) :name(name) {
    cnt[0] = cnt[1] = 0;
}

Node::Node(Cell *c) {
    cell = c;
    next = prev = nullptr;
}

void Input::LoadCellAndNet(char *argv[]) {
    // ------  Load Cell -------
    ifstream cellfile(argv[2]);

    // CellFile format: c78 1
    string netName, cellName;
    int size;
    while (cellfile >> cellName >> size) {
        Cell *newCell = new Cell(cellName, size);
        totalSize += size;
        cells.push_back(newCell);
        handles[cellName] = newCell;
    }
    cout << "Debug: Finish loading cell\n";

    // ------  Load Net  -------
    ifstream netfile(argv[1]);

    // NetFile format: NET n1 {c1 c2 c3}
    // tmp: NET, {
    string tmp;
    while (netfile >> tmp >> netName >> tmp) {
        // create a newNet
        Net* newNet = new Net(netName);

        // record the connected cells
        while (netfile >> cellName) {
            // load '}' means that it's the end
            if (cellName[0] == '}') break;
            
            // find out the corresponding cell
            Cell* cell = handles[cellName];
            newNet->cells.push_back(cell);
            cell->nets.push_back(newNet);
        }
        nets.push_back(newNet);
    }
    cout << "Debug: Finish loading net\n";


    // calculate the balance
    balance = totalSize / 10.0;
}

