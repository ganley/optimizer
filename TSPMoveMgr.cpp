#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

#include <math.h>
#include <assert.h>

#include "IOptimizer.h"
#include "TSPMove.h"
#include "TSPMoveMgr.h"

using namespace std;



TSPMoveMgr::TSPMoveMgr(const std::string& filename)
:   _size(0)
{
    // Read the TSP instance. This will fail non-gracefully on the instances that
    // are not specified as a set of points.
    ifstream in(filename.c_str());
    bool gotCoords = false;
    while (!gotCoords) {
        string token;
        in >> token;
        if (token.substr(0, 9) == "DIMENSION") {
            in >> token;
            if (token == ":") {
                in >> _size;
            } else {
                _size = atol(token.c_str());
            }
            assert(_size > 2);
            cerr << "Got DIMENSION: " << _size << endl;
        } else if (token.substr(0, 4) == "NAME") {
            in >> token;
            if (token == ":") {
                in >> _name;
            } else {
                _name = token;
            }
            cerr << "Got NAME: " << _name << endl;
        } else if (token == "NODE_COORD_SECTION") {
            cerr << "Begin NODE_COORD_SECTION" << endl;
            assert(_size > 0);

            _x = new double[_size];
            _y = new double[_size];
            for (int i = 0; i < _size; ++i) {
                int index;
                double x;
                double y;
                in >> index >> x >> y;
                assert(index >= 1 && index <= _size);
                _x[index - 1] = double(x);
                _y[index - 1] = double(y);
            }
            in >> token;
            assert(token == "EOF");

            cerr << "Done NODE_COORD_SECTION" << endl;

            gotCoords = true;
        }
    }
    in.close();

    assert(gotCoords);


    // create an arbitrary tour
    _tour = new int[_size];
    for (int i = 0; i < _size; ++i) {
        _tour[i] = next(i);
    }

    _cost = computeScore();

    // DEBUG
    cerr << "initial tour:\n";
    for (int i = 0, n = 0; i < _size; ++i, n = _tour[n]) {
        cerr << n << " ";
    }
    cerr << endl;
    cerr << "cost=" << _cost << endl;
}



TSPMoveMgr::~TSPMoveMgr()
{
    delete[] _x;
    delete[] _y;
}



void
TSPMoveMgr::generateMove(TSPMove* move)
{
    // pick a random pair that are different and not neighbors
    do {
        move->_a = rand() % _size;
        move->_b = rand() % _size;
    } while (move->_a == move->_b || _tour[move->_a] == move->_b || _tour[move->_b] == move->_a);
}



double
TSPMoveMgr::proposeMove(const TSPMove* move)
{
    const int a = move->_a;
    const int aNext = _tour[move->_a];
    const int b = move->_b;
    const int bNext = _tour[move->_b];

    // the edges (a,aNext) and (b,bNext) will be removed and replaced with
    // the edges (a,b) and (aNext,bNext)
    const double newedges = L2Dist(_x[a], _y[a], _x[b], _y[b])
                          + L2Dist(_x[aNext], _y[aNext], _x[bNext], _y[bNext]);
    const double oldedges = L2Dist(_x[a], _y[a], _x[aNext], _y[aNext])
                          + L2Dist(_x[b], _y[b], _x[bNext], _y[bNext]);

    return newedges - oldedges;
}



// FIX cache the last proposal
double
TSPMoveMgr::makeMove(const TSPMove* move)
{
    // compute the move's cost
    const double delta = proposeMove(move);
    _cost += delta;

    // modify the tour to implement the move. this involves removing the edges
    // (a,aNext) and (b,bNext), adding the edges (a,b) and (aNext,bNext), and
    // reversing the section of the tour between aNext and bNext.
    const int a = move->_a;
    const int aNext = _tour[move->_a];
    const int b = move->_b;
    const int bNext = _tour[move->_b];
    int x = aNext;
    int n1 = _tour[x];
    while (n1 != bNext) {
        const int n2 = _tour[n1];
        _tour[n1] = x;
        x = n1;
        n1 = n2;
    }
    _tour[a] = b;
    _tour[aNext] = bNext;

    return delta;
}



double
TSPMoveMgr::getScore()
{
    return _cost;
}



double
TSPMoveMgr::computeScore() const
{
    double cost = 0.0;
    for (int i = 0; i < _size; ++i) {
        const int j = _tour[i];
        cost += L2Dist(_x[i], _y[i], _x[j], _y[j]);
    }

    return cost;
}



unsigned int
TSPMoveMgr::getProblemSize()
{
    return _size;
}



void
TSPMoveMgr::debug()
{
    cerr << "tour:";
    for (int i = 0, n = 0; i < _size; ++i, n = _tour[n]) {
        cerr << " " << n;
    }
    cerr << endl;

    cerr << "alleged cost: " << getScore() << endl;
    cerr << "scratch cost: " << computeScore() << endl;
}



double
TSPMoveMgr::L2Dist(const double x0,
                   const double y0,
                   const double x1,
                   const double y1)
{
    const double deltaX = abs(x0 - x1);
    const double deltaY = abs(y0 - y1);

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

