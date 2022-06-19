#include <iostream>
#include <chrono>

class Timer {
    private:
        std::chrono::high_resolution_clock::time_point startTime;
        std::chrono::high_resolution_clock::time_point endTime;
        std::chrono::high_resolution_clock::time_point startI;
        std::chrono::high_resolution_clock::time_point endI;
        double maxI = 0;

    public:
        Timer() {
            startTime = std::chrono::high_resolution_clock::now();
            std::cout << "Timer Started!\n";
        }

        void setS() {
            startI = std::chrono::high_resolution_clock::now();
        }

        double setE() {
            endI = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = endI - startI;
            if (diff.count() > maxI) {
                maxI = diff.count();
                std::cerr << maxI << " s\n";
            }
            return diff.count();
        }

        void getTime() {
            endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = endTime - startTime;
            std::cout << diff.count() << " s\n";
        }

        bool over() {
            endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = endTime - startTime;

            return diff.count() > 3590;
        }
};