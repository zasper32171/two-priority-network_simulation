#include "Simulator.h"

#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char** argv)
{
    short n = atoi(argv[0]);
    char* filename = argv[1];

    FILE* log = fopen(filename, "w");

    for (double load = 0.1; load <= 1.0; load += 0.1)
    {
        for (short t = 0; t < 10; t++)
        {
            Simulator simulator = Simulator(n, load);

            simulator.setTransitionTime(1000);
            simulator.setCycleTime(100000);
            simulator.simulate();

            simulator.writeResults(log);
//          simulator.showResults();
        }
    }

    fclose(log);

    return 0;
}
