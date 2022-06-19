#include <iostream>
#include <string>
// #include "Reader.hpp"
#include "Floorplan.hpp"
#include <ctime>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 5) {
        cout << "missing some parameters\n";
        return 0;
    }

    clock_t IOtime, Exectime;
    clock_t loadTime1, loadTime2;
    clock_t ecTime1, ecTime2;
    clock_t outputTime1, outputTime2; 

    clock_t StartTime, FinishTime;

    StartTime = clock();
    loadTime1 = clock();
    Input *_input = new Input();
    _input->ReadAllFile(argv);
    cout << "Debug: Finish loading three files\n";
    loadTime2 = clock();

    ecTime1 = clock();
    Floorplan FP(_input);
    FP.Algorithm();
    cout << "Debug: Finish Floorplanning\n";
    ecTime2 = clock();

    outputTime1 = clock();
    FP.outputResult(argv[4]);
    cout << "Debug: Finish writing the result\n";
    outputTime2 = clock();
    FinishTime = clock();
   
    cout << "Time: load time usage = " << double(loadTime2 - loadTime1) / CLOCKS_PER_SEC << endl;
    cout << "Time: exec time usage = " << double(ecTime2 - ecTime1) / CLOCKS_PER_SEC << endl;
    cout << "Time: output time usage = " << double(outputTime2 - outputTime1) / CLOCKS_PER_SEC << endl;
    cout << "Time: total time usage = " << double(FinishTime - StartTime) / CLOCKS_PER_SEC << endl;

    return 0;
}