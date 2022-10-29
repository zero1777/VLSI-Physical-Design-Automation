#include <iostream>
// #include "LCN.hpp"
#include "Partition.hpp"
#include <ctime>

using namespace std;

int main(int argc, char *argv[]) {
    // verify the needed parameter
    if (argc < 4) {
        cout << "There are some missed parameters.";
        return 0;
    }

    int times = 1;
    int min = -1;
    int seed;
    
    for (int i=0; i<times; i++) {
        clock_t loadTime1, loadTime2;
        clock_t execTime1, execTime2;
        clock_t outputTime1, outputTime2;

        // cout << "Test: New Round --------------- \n";
        // load the cell & net file
        loadTime1 = clock();
        Input *_input = new Input();
        _input->LoadCellAndNet(argv);
        loadTime2 = clock();

        cout << "Debug: Finish loading cell and net\n";
        cout << "Time: Input time = " << double(loadTime2-loadTime1) / CLOCKS_PER_SEC << "\n"; 


        // run FM (Fiduccia–Mattheyses) algorithm
        execTime1 = clock();
        FM_Algorithm FM(_input);
        FM.Run();
        execTime2 = clock();

        // cout << "Test: seed = " << FM.testSeed << "\n";
        if (min == -1) {
            min = FM.minCutSize;
            seed = FM.testSeed;
        }
        else {
            if (min > FM.minCutSize) {
                seed = FM.testSeed;
                min = FM.minCutSize;
            }
        }
        

        cout << "Debug: Finish running FM algorithm\n";
        
        // write the final result to the output file
        outputTime1 = clock();
        FM.OutputResult(argv[3]);
        outputTime2 = clock();
        
        cout << "Time: Output time = " << double(outputTime2-outputTime1) / CLOCKS_PER_SEC << "\n";
        cout << "Debug: Finish writing the result to the output file\n";

        cout << "Time: I/O time = " << (double(loadTime2-loadTime1) + double(outputTime2-outputTime1)) / CLOCKS_PER_SEC << "\n";
        cout << "Time: Execution time = " << double(execTime2-execTime1) / CLOCKS_PER_SEC << "\n";
    }

    // FM.OutputTest(argv[3]);
    cout << "Debug: Finish writing the test result\n";

    cout << "Debug: Program done :D\n";
    // cout << "Result: Best seed = " << seed << "\n"; 
    // cout << "Result: Best cutSize = " << min << "\n"; 



    return 0;
}