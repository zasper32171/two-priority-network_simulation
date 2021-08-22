#include "Simulator.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include <vector>

using namespace std;

Simulator::Simulator(short network_size, double offered_load, double high_prio_ratio)
{
    _size = network_size;
    _port = 0x1 << network_size;
    _load = offered_load;
    _ratio = high_prio_ratio;

    _trans = 0;
    _cycle = 10000;

    _arrival[0] = 0;
    _arrival[1] = 0;
    _block[0] = 0;
    _block[1] = 0;
    _departure[0] = 0;
    _departure[1] = 0;
    _delay[0] = 0;
    _delay[1] = 0;

    _min = new Packet***[_size];

    for (short i = 0; i < _size; i++)
    {
        _min[i] = new Packet**[_port];

        for (short j = 0; j < _port; j++)
        {
            _min[i][j] = new Packet*[_port];
            _min[i][j][1] = NULL;
            _min[i][j][0] = NULL;
        }
    }

    srand(time(NULL));
}

Simulator::~Simulator()
{
    for (short i = 0; i < _size; i++)
    {
        for (short j = 0; j < _port; j++)
        {
            if (_min[i][j][1] != NULL)
                delete _min[i][j][1];

            if (_min[i][j][0] != NULL)
                delete _min[i][j][0];

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
    printf("Pb(h) = %f\n", (double)_block[1] / _arrival[1]);
    printf("Pb(l) = %f\n", (double)_block[0] / _arrival[0]);

    printf("Th = %f\n", (double)(_departure[1] + _departure[0]) / _cycle / _port);
    printf("Th(h) = %f\n", (double)_departure[1] / _cycle / _port / (_load * _ratio));
    printf("TH(l) = %f\n", (double)_departure[0] / _cycle / _port / (_load * (1 - _ratio)));

    printf("D(h) = %f\n", (double)_delay[1] / _departure[1] / _size);
    printf("D(l) = %f\n", (double)_delay[0] / _departure[0] / _size);
}

void Simulator::writeResults(FILE* log)
{
    fprintf(log, "%f,", (double)_block[1] / _arrival[1]);
    fprintf(log, "%f,", (double)_block[0] / _arrival[0]);

    fprintf(log, "%f,", (double)(_departure[1] + _departure[0]) / _cycle / _port);
    fprintf(log, "%f,", (double)_departure[1] / _cycle / _port / (_load * _ratio));
    fprintf(log, "%f,", (double)_departure[0] / _cycle / _port / (_load * (1 - _ratio)));

    fprintf(log, "%f,", (double)_delay[1] / _departure[1] / _size);
    fprintf(log, "%f\n", (double)_delay[0] / _departure[0] / _size);
}

void Simulator::clearResults()
{
    _arrival[0] = 0;
    _arrival[1] = 0;
    _block[0] = 0;
    _block[1] = 0;
    _departure[0] = 0;
    _departure[1] = 0;
    _delay[0] = 0;
    _delay[1] = 0;
}

void Simulator::arrival()
{
    for (short j = 0; j < _port; j++)
    {
        if (rand() < _load * (RAND_MAX + 1)) {
            short dst = floor((double)rand() / (RAND_MAX + 1) * _port);

            if (rand() < _ratio * (RAND_MAX + 1))
            {
                if (_min[0][j][1] == NULL) {
                    _min[0][j][1] = new Packet(j, dst);
                } else {
                    _block[1]++;
                }
                _arrival[1]++;
            }
            else
            {
                if (_min[0][j][0] == NULL) {
                    _min[0][j][0] = new Packet(j, dst);
                } else {
                    _block[0]++;
                }
                _arrival[0]++;
            }
        }
    }
}

void Simulator::propagate()
{
    vector <short> contend[_port][2];
    bool transmit[_port] = {};

    for (short i = _size - 2; i >= 0; i--)
    {
        for (short j = 0; j < _port; j++)
        {
            if (_min[i][j][1] != NULL) {
                short next = _min[i][j][1]->_next;
                if (_min[i+1][next][1] == NULL)
                    contend[next][1].push_back(j);
            }

            if (_min[i][j][0] != NULL) {
                short next = _min[i][j][0]->_next;
                if (_min[i+1][next][0] == NULL)
                    contend[next][0].push_back(j);
            }
        }

        for (short j = 0; j < _port; j++)
        {
            if (contend[j][1].empty())
                continue;

            if (contend[j][1].size() == 1)
            {
                short prev = contend[j][1][0];

                assert(_min[i+1][j][1] == NULL);

                _min[i+1][j][1] = _min[i][prev][1];
                _min[i][prev][1] = NULL;

                transmit[prev] = true;
            }
            else
            {
                short p = floor((double)rand() / (RAND_MAX + 1) * contend[j][1].size());
                short prev = contend[j][1][p];

                assert(_min[i+1][j][1] == NULL);

                _min[i+1][j][1] = _min[i][prev][1];
                _min[i][prev][1] = NULL;

                transmit[prev] = true;
            }
        }

        for (short j = 0; j < _port; j++) {
            for (size_t n = 0; n < contend[j][0].size(); n++) {
                if (transmit[contend[j][0][n]] == true) {
                    contend[j][0].erase(contend[j][0].begin() + n);
                    n--;
                }
            }
        }

        for (short j = 0; j < _port; j++)
        {
            if (contend[j][0].empty())
                continue;

            if (contend[j][0].size() == 1)
            {
                short prev = contend[j][0][0];

                assert(_min[i+1][j][0] == NULL);

                _min[i+1][j][0] = _min[i][prev][0];
                _min[i][prev][0] = NULL;
            }
            else
            {
                short p =  floor((double)rand() / (RAND_MAX + 1) * contend[j][1].size());
                short prev = contend[j][0][p];

                assert(_min[i+1][j][0] == NULL);

                _min[i+1][j][0] = _min[i][prev][0];
                _min[i][prev][0] = NULL;
            }
        }

        for (short j = 0; j < _port; j++) {
            contend[j][0].clear();
            contend[j][1].clear();
            transmit[j] = false;
        }
    }
}

void Simulator::departure()
{
    for (short j = 0; j < _port; j++)
    {
        if (_min[_size-1][j][1] != NULL)
        {
            _delay[1] += _min[_size-1][j][1]->_delay;

            delete _min[_size-1][j][1];
            _min[_size-1][j][1] = NULL;

            _departure[1]++;
        }
        else if (_min[_size-1][j][0] != NULL)
        {
            _delay[0] += _min[_size-1][j][0]->_delay;

            delete _min[_size-1][j][0];
            _min[_size-1][j][0] = NULL;

            _departure[0]++;
        }
    }
}

void Simulator::incrementDelay()
{
    for (short i = 0; i < _size; i++) {
        for (short j = 0; j < _port; j++)
        {
            if (_min[i][j][1] != NULL)
                _min[i][j][1]->_delay++;

            if (_min[i][j][0] != NULL)
                _min[i][j][0]->_delay++;
        }
    }
}

void Simulator::determineNext()
{
    for (short i = 0; i < _size; i++) {
        for (short j = 0; j < _port; j++)
        {
            if (_min[i][j][1] != NULL)
                _min[i][j][1]->_next = (_min[i][j][1]->_src >> (i + 1)) |
                    (_min[i][j][1]->_dst >> (_size - (i + 1)) << (_size - (i + 1)));

            if (_min[i][j][0] != NULL)
                _min[i][j][0]->_next = (_min[i][j][0]->_src >> (i + 1)) |
                    (_min[i][j][0]->_dst >> (_size - (i + 1)) << (_size - (i + 1)));
        }
    }
}

