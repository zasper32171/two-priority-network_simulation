#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdio.h>

class Simulator
{
public:
    Simulator(short, double, double);
    ~Simulator();

    void setTransitionTime(long cycle) { _trans = cycle; }
    void setCycleTime(long cycle) { _cycle = cycle; }
    void simulate();
    void showResults();
    void writeResults(FILE*);
    void clearResults();

private:
    void arrival();
    void propagate();
    void departure();
    void incrementDelay();
    void determineNext();

    class Packet
    {
    public:
        Packet(short src, short dst): _src(src), _dst(dst), _delay(0) {}

        short   _src;
        short   _next;
        short   _dst;
        short   _delay;
    };

    Packet****   _min;
    short       _size;
    short       _port;
    double      _load;
    double      _ratio;

    long        _trans;
    long        _cycle;

    long        _arrival[2];
    long        _block[2];
    long        _departure[2];
    long        _delay[2];
};

#endif // SIMULATOR_H
