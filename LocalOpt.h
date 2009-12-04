

#if !defined(LOCALOPT_H)
#define LOCALOPT_H

#include <iostream>

#include "IOptimizer.h"



template<class MoveType,
         class CostType    = double,
         class MoveMgrType = IMoveMgr<MoveType, CostType> >
class LocalOpt : public IOptimizer<MoveType, CostType, MoveMgrType> {
  public:
    virtual void            optimize(MoveMgrType* moveMgr);
};




template<class MoveType, class CostType, class MoveMgrType>
void
LocalOpt<MoveType, CostType, MoveMgrType>::optimize(MoveMgrType* moveMgr)
{
    const int missThreshold = 10000;
    int       misses        = missThreshold;

    while (true) {
        MoveType move;
        moveMgr->generateMove(move);

        const CostType cost = moveMgr->proposeMove(move);

        if (cost < 0) {
            moveMgr->makeMove(move);
            misses = missThreshold;
        } else {
            if (--misses < 0) {
                return;
            }
        }

        std::cout << "Score = " << moveMgr->getScore() << "\n";
   }
}



#endif
