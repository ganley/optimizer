
#if !defined(TSPMOVEMGR_H)
#define TSPMOVEMGR_H

#include "IOptimizer.h"
#include "TSPMove.h"



class TSPMoveMgr : public IMoveMgr<TSPMove, double> {
  public:
    TSPMoveMgr(const std::string& filename);
    ~TSPMoveMgr();

    virtual void            generateMove(TSPMove* move);
    virtual double          proposeMove(const TSPMove* move);
    virtual double          makeMove(const TSPMove* move);
    virtual double          getScore();
    virtual unsigned int    getProblemSize();

    virtual void            debug();

  private:
    int                     prev(const int i) const;
    int                     next(const int i) const;
    double                  computeScore() const;
    static double           L2Dist(const double x0, const double y0,
                                   const double x1, const double y1);

  private:
    int         _size;
    double*     _x;
    double*     _y;
    int*        _tour;    // _tour[i] is the next vertex after i in the tour
    double      _cost;
    std::string _name;
};



inline int
TSPMoveMgr::prev(const int i) const
{
    return i == 0 ? _size - 1 : i - 1;
}



inline int
TSPMoveMgr::next(const int i) const
{
    const int n = i + 1;
    return n == _size ? 0 : n;
}



#endif

