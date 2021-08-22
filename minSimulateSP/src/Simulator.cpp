#include "Simulator.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include <vector>

using namespace std;

Simulator::Simulator(short network_size, double offered_load)
{
    _size = network_size;
    _port = 0x1 << network_size;
    _load = offered_load;

    _trans = 0;
    _cycle = 10000;

    _arrival = 0;
    _block = 0;
    _departure = 0;
    _delay = 0;

    _min = new Packet**[_size];

    for (short i = 0; i < _size; i++)
    {
        _min[i] = new Packet*[_port];

        for (short j = 0; j < _port; j++)
            _min[i][j] = NULL;
    }

    srand(time(NULL));
}

Simulator::~Simulator()
{
    for (short i = 0; i < _size; i++)
    {
        for (short j = 0; j < _port; j++)
        {
            if (_min[i][j] != NULL)
                delete _min[i][j];
        }
        delete _min[i];
    }
    delete _min;
}

void Simulator::simulate()
{
    for (long t = 0; t < _trans; t++)
    {
        incrementDelay();
        departure();
        propagate();
        arrival();
        determineNext();
    }

    clearResults();

    for (long t = 0; t < _cycle; t++)
    {
        incrementDelay();
        departure();
        propagate();
        arrival();
        determineNext();
    }
}

void Simulator::showResults()
{
    printf("Pb(h) = %f\n", (double)_block / _arrival);
    printf("Th = %f\n", (double)_departure / _cycle / _port / _load);
    printf("D(h) = %f\n", (double)_delay / _departure / _size);
}

void Simulator::writeResults(FILE* log)
{
    fprintf(log, "%f,", (double)_block / _arrival);
    fprintf(log, "%f,", (double)_departure / _cycle / _port / _load);
    fprintf(log, "%f\n", (double)_delay / _departure / _size);
}

void Simulator::clearResults()
{
    _arrival = 0;
    _block = 0;
    _departure = 0;
    _delay = 0;
}

void Simulator::arrival()
{
    for (short j = 0; j < _port; j++)
    {
        if (rand() < _load * (RAND_MAX + 1))
		{
            if (_min[0][j] == NULL) {
				short dst = floor((double)rand() / (RAND_MAX + 1) * _port);
                _min[0][j] = new Packet(j, dst);
			}
            else
                _block++;

            _arrival++;
        }
    }
}

void Simulator::propagate()
{
    vector <short> contend[_port];

    for (short i = _size - 2; i >= 0; i--)
    {
        for (short j = 0; j < _port; j++)
        {
            if (_min[i][j] != NULL) {
                short next = _min[i][j]->_next;
                if (_min[i+1][next] == NULL)
                    contend[next].push_back(j);
            }
        }

        for (short j = 0; j < _port; j++)
        {
            if (contend[j].empty())
                continue;

            if (contend[j].size() == 1)
            {
                short prev = contend[j][0];

                assert(_min[i+1][j] == NULL);

                _min[i+1][j] = _min[i][prev];
                _min[i][prev] = NULL;
            }
            else
            {
                short p = floor((double)rand() / (RAND_MAX + 1) * contend[j].size());
                short prev = contend[j][p];

                assert(_min[i+1][j] == NULL);

                _min[i+1][j] = _min[i][prev];
                _min[i][prev] = NULL;
            }
        }

        for (short j = 0; j < _port; j++)
            contend[j].clear();
    }
}

void Simulator::departure()
{
    for (short j = 0; j < _port; j++)
    {
        if (_min[_size-1][j] != NULL)
        {
            _delay += _min[_size-1][j]->_delay;

            delete _min[_size-1][j];
            _min[_size-1][j] = NULL;

            _departure++;
        }
    }
}

void Simulator::incrementDelay()
{
    for (short i = 0; i < _size; i++) {
        for (short j = 0; j < _port; j++) {
            if (_min[i][j] != NULL)
                _min[i][j]->_delay++;
        }
    }
}

void Simulator::determineNext()
{
    for (short i = 0; i < _size; i++) {
        for (short j = 0; j < _port; j++) {
            if (_min[i][j] != NULL)
                _min[i][j]->_next = (_min[i][j]->_src >> (i + 1)) |
                    (_min[i][j]->_dst >> (_size - (i + 1)) << (_size - (i + 1)));
        }
    }
}
