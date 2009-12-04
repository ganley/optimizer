
#if !defined(IOPTIMIZER_H)
#define IOPTIMIZER_H



//******************************************************************************
// IMoveMgr
//
// This abstract base class defines the IMoveMgr interface, to be implemented
// by a move manager for a Monte Carlo optimization algorithm. The problem state
// is generally stored within the move manager.
//******************************************************************************
template<class MoveType,
	     class CostType = double>
class IMoveMgr {
  public:
    // Generate a move. For problems where there are multiple move
    // types, I suggest implementing a move base class from which
    // the moves of various types are derived. This use model is
    // why this and the other methods are passed a move pointer
    // rather than a reference.
    virtual void            generateMove(MoveType* move)      = 0;

    // Compute the delta-cost of a proposed move. This is new
    // score minus old score, so a negative value means a reduction
    // in the score. The optimizers I've written assume lower score
    // is better, so if your problem is a maximization problem, it
    // is probably best to invert your score.
    virtual CostType	    proposeMove(const MoveType* move) = 0;

    // Make a move, and return the delta-cost incurred. It's up to
    // you whether you compute that cost by calling proposeMove, but
    // note that typically this move will be the one for which
    // proposeMove was just called, so caching its return value may
    // be a win.
    virtual CostType	    makeMove(const MoveType* move)    = 0;

    // Get the current total score. My optimizers assume that the
    // move manager is tracking its score internally, i.e. that this
    // operation is cheap.
    virtual CostType	    getScore()		                  = 0;

    // Get the problem size. Some optimizers set parameters as a
    // function of problem size. Exactly what 'size' means depends
    // on the problem.
    virtual unsigned int    getProblemSize()                  = 0;

    // Debugging harness. This is just a pass-through so that you
    // can easily add debug hooks to your move manager. The code
    // I've written never calls this.
    virtual void            debug() {}
};



//******************************************************************************
// IOptimizer
//
// This abstract base class defines the IOptimizer interface, to be implemented
// by a Monte Carlo optimization algorithm.
//******************************************************************************
template<class MoveType,
         class CostType    = double,
         class MoveMgrType = IMoveMgr<MoveType, CostType> >
class IOptimizer {
  public:
    virtual void            optimize(MoveMgrType* moveMgr) = 0;
};



#endif

