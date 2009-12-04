// Move manager for geometric TSP

#if !defined(TSPMOVE_H)
#define TSPMOVE_H

#include "IOptimizer.h"



class TSPMove {
  public:
    TSPMove(int a = 0,
            int b = 0);

    int _a;
    int _b;
};



inline
TSPMove::TSPMove(int a,
                 int b)
:   _a(a),
    _b(b)
{
}



#endif

