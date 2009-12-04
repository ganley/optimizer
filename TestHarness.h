// The world's dumbest (well, maybe second- or third-dumbest) sorting algorithm.

#if !defined(TESTHARNESS_H)
#define TESTHARNESS_H

#include "IOptimizer.h"



class Move {
  public:
    Move(int from = 0,
         int to   = 0);

    int _from;
    int _to;
};



class TestHarnessMoveMgr : public IMoveMgr<Move, int> {
  public:
    TestHarnessMoveMgr(unsigned int problemSize);

    virtual void            generateMove(Move* move);
    virtual int             proposeMove(const Move* move);
    virtual int             makeMove(const Move* move);
    virtual int             getScore();
    virtual unsigned int    getProblemSize();

    virtual void            debug();

  private:
    int     _size;
    int*    _data;
};



inline
Move::Move(int from,
           int to)
:   _from(from),
    _to(to)
{
}



#endif

