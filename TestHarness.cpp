#include <algorithm>
#include <iostream>
#include <utility>

#include <assert.h>

#include "TestHarness.h"

using std::max;
using std::min;
using std::swap;



TestHarnessMoveMgr::TestHarnessMoveMgr(unsigned int problemSize)
:   _size(problemSize),
    _data(new int[_size])
{
    assert(_size > 5);

    srand(5241999);

    // Fill the array with ascending integers, and then shuffle them.
    for (int i = 0; i < _size; i++) {
        _data[i] = i;
    }

    for (int i = 1; i < _size; i++) {
        swap(_data[i], _data[rand() % i]);
    }
}



void
TestHarnessMoveMgr::generateMove(Move* move)
{
    do {
        move->_from = rand() % _size;
        move->_to   = rand() % _size;
    } while (move->_from == move->_to);
}



int
TestHarnessMoveMgr::proposeMove(const Move* move)
{
    int cost = 0;

    const int lo = min(move->_from, move->_to);
    const int hi = max(move->_from, move->_to);

    const int loVal = min(_data[lo], _data[hi]);
    const int hiVal = max(_data[lo], _data[hi]);

    for (int i = lo + 1; i < hi; i++) {
        if (_data[i] > loVal && _data[i] < hiVal) {
            cost += 2;
        }
    }

    cost += 1;  // for the pair [lo,hi]

    return _data[lo] < _data[hi] ? cost : -cost;
}



int
TestHarnessMoveMgr::makeMove(const Move* move)
{
    const int cost = proposeMove(move);

    swap(_data[move->_from], _data[move->_to]);

    return cost;
}



int
TestHarnessMoveMgr::getScore()
{
    int cost = 0;

    for (int i = 0; i < _size; i++) {
        for (int j = i + 1; j < _size; j++) {
            if (_data[i] > _data[j]) {
                cost++;
            }
        }
    }

    return cost;
}



unsigned int
TestHarnessMoveMgr::getProblemSize()
{
    return _size;
}



void
TestHarnessMoveMgr::debug()
{
    for (int i = 0; i < _size; i++) {
        std::cout << _data[i] << " ";
    }
    std::cout << "\n";
}

