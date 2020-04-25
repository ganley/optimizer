

#if !defined(ANNEALER_H)
#define ANNEALER_H

#include <assert.h>
#include <math.h>

#include "IOptimizer.h"



template<class MoveType,
         class CostType    = double,
         class MoveMgrType = IMoveMgr<MoveType, CostType> >
class Annealer : public IOptimizer<MoveType, CostType, MoveMgrType> {
  public:
    virtual void            optimize(MoveMgrType* moveMgr);

  private:
    double                  measureTemp();
    void                    equilibrate(const double temp,
                                        double&      meanCost,
                                        double&      costStdDev,
                                        double&      deltaCostStdDev,
                                        double&      acceptRatio);
    double                  project(const int       n,
                                    const double*   x,
                                    const CostType* y) const;
    void                    seedRand(unsigned int seed);
    double                  getRand();

    MoveMgrType*            _moveMgr;
};



// This is the main routine.
template<class MoveType, class CostType, class MoveMgrType>
void
Annealer<MoveType, CostType, MoveMgrType>::optimize(MoveMgrType* moveMgr)
{
    // Don't consider stopping until after this many equilibria
    const int       minEquilsKnob           = 5;

    // Don't consider stopping due to convergence unless we've made this much improvement
    // over the initial cost.
    const double    requiredImprovementKnob = 0.1;

    // Regardless of the convergence criterion, stop if this many equilibria go by
    // without seeing a new best cost. This could probably be lower.
    const int       equilsSinceBestKnob     = 100;

    seedRand(5241999);

    _moveMgr = moveMgr;

    double          temp    = measureTemp();
    CostType        best    = _moveMgr->getScore();
    const CostType  first   = best;
    double          tempHistory[minEquilsKnob];
    CostType        costHistory[minEquilsKnob];

    // Repeat until we exceed equilsSinceBestKnob equilibria with no new best score.
    // The convergence stop criterion will break out of this loop.
    int equilsSinceBest = equilsSinceBestKnob;
    for (int equils = 0; best > 0 && equilsSinceBest-- > 0; ++equils) {

        // Do an equilibrium.
        double meanCost;
        double costStdDev;
        double deltaCostStdDev;
        double acceptRatio;
        equilibrate(temp, meanCost, costStdDev, deltaCostStdDev, acceptRatio);

        // If we have a new best score, store it and reset equilsSinceBest
        const CostType c = _moveMgr->getScore();
        if (c < best) {
            best = c;
            equilsSinceBest = equilsSinceBestKnob;
        }

        std::cout << "t=" << temp << " c=" << _moveMgr->getScore() << " ";

        // Once we get past the minimum number of equilibria, check for stop criterion.
        // This is done by fitting a line through the last several (temp,cost) points.
        // When the intercept of that line is essentially equal to the current score, stop.
        // We also require a certain amount of improvement to have happened first, since
        // this stop criterion often false-alarms at the very beginning otherwise.
        const int ix = equils % minEquilsKnob;
        costHistory[ix] = c;
        tempHistory[ix] = temp;
        if (equils > minEquilsKnob) {
            const double intercept = project(minEquilsKnob, tempHistory, costHistory);
            std::cout << "s=" << intercept << "\n";
            if (abs(intercept - c) < 0.00001 && c < first * (1.0 - requiredImprovementKnob)) {
                break;
            }
        } else {
            std::cout << "\n";
        }

        // There are more sophisticated cooling schedules, but I haven't done the
        // research to figure out if the ones I know are trade secrets or prior art.
        temp *= 0.95;
    }

    std::cout << "t=" << temp << " c=" << _moveMgr->getScore() << "   --   ";
}



// Measure the starting temperature. This is done by performing a binary search until
// we find the temperature at which we would make a roughly equal number of uphill vs.
// downhill moves.
template<class MoveType, class CostType, class MoveMgrType>
double
Annealer<MoveType, CostType, MoveMgrType>::measureTemp()
{
    const int movesPerTempKnob  = 100;
    const int movesPerTemp      = movesPerTempKnob * _moveMgr->getProblemSize();
    const int halfMovesPerTemp  = movesPerTemp / 2;

    double hiTemp   = 10000000.0;
    double loTemp   = 0.00001;
    while (hiTemp - loTemp > 1.0) {
        const double temp = (hiTemp + loTemp) / 2.0;
        int accepted = 0;
        for (int attempts = 0; attempts < movesPerTemp; ++attempts) {
            MoveType move;
            _moveMgr->generateMove(&move);
            const CostType  deltaCost       = _moveMgr->proposeMove(&move);
            const CostType  absDeltaCost    = abs(deltaCost);
            const double    boltzmann       = exp(-absDeltaCost / temp);
            if (deltaCost < 0 || getRand() < boltzmann) {
                ++accepted;
            }
        }

        std::cerr << "t=" << temp << " acc=" << accepted << " of " << movesPerTemp << " - going ";

        if (accepted > halfMovesPerTemp) {
            hiTemp = temp;
            std::cerr << "down" << std::endl;
        } else {
            loTemp = temp;
            std::cerr << "up" << std::endl;
        }
    }

    return hiTemp;
}



// Do an equilibrium. Standard simulated annealing Markov chain, gathering statistics
// as we go.
template<class MoveType, class CostType, class MoveMgrType>
void
Annealer<MoveType, CostType, MoveMgrType>::equilibrate(const double temp,
                                                       double&      meanCost,
                                                       double&      costStdDev,
                                                       double&      deltaCostStdDev,
                                                       double&      acceptRatio)
{
    const double maxAcceptKnob    = 10.0;
    const double maxAttemptKnob   = 100.0;

    double       totalCost        = 0.0;
    double       totalCostSq      = 0.0;
    double       totalDeltaCost   = 0.0;
    double       totalDeltaCostSq = 0.0;

    int          attempts         = 0;
    int          acceptances      = 0;
    const int    maxAttempts      = int(_moveMgr->getProblemSize() * maxAttemptKnob);
    const int    maxAcceptances   = int(_moveMgr->getProblemSize() * maxAcceptKnob);

    CostType     curr_cost        = _moveMgr->getScore();

    for (; attempts < maxAttempts && acceptances < maxAcceptances; attempts++) {        
        MoveType move;
        _moveMgr->generateMove(&move);

        const CostType deltaCost    = _moveMgr->proposeMove(&move);
        const CostType absDeltaCost = abs(deltaCost);
        const double   boltzmann    = exp(-absDeltaCost / temp);

        const double   prob         = absDeltaCost * boltzmann;
        totalDeltaCost              += prob;
        totalDeltaCostSq            += absDeltaCost * prob;

        const double effProb = deltaCost < 0.0 ? 1.0 : boltzmann;
        totalCost += curr_cost + effProb * deltaCost;
        const double newCost = curr_cost + deltaCost;
        totalCostSq += ((1.0 - effProb) * (curr_cost * curr_cost) + effProb * newCost * newCost);

        if (deltaCost < 0 || getRand() < boltzmann) {
            _moveMgr->makeMove(&move);

            curr_cost += deltaCost;
            assert(curr_cost == _moveMgr->getScore()); // FIX debugging only EXP

            acceptances++;
        }
    }

    const double n = double(attempts);
    meanCost = double(totalCost) / n;
    costStdDev = (totalCostSq / n) - ((totalCost * totalCost) / (n * n));
    acceptRatio = double(acceptances) / n;
}



// Compute the y-intercept of a line fit via least-squares
template<class MoveType, class CostType, class MoveMgrType>
double
Annealer<MoveType, CostType, MoveMgrType>::project(const int        n,
                                                   const double*    x,
                                                   const CostType*  y) const
{
    double sumX = 0.0;
    double sumXsq = 0.0;
    double sumY = 0.0;
    double sumXY = 0.0;
    for (int i = 0; i < n; ++i) {
        sumX += x[i];
        sumXsq += x[i] * x[i];
        sumY += y[i];
        sumXY += x[i] * y[i];
    }

    return (sumY * sumXsq - sumX * sumXY) / (n * sumXsq - sumX * sumX);
}



// FIX use a better RNG.
template<class MoveType, class CostType, class MoveMgrType>
void
Annealer<MoveType, CostType, MoveMgrType>::seedRand(unsigned int seed)
{
    srand(seed);
}



template<class MoveType, class CostType, class MoveMgrType>
double
Annealer<MoveType, CostType, MoveMgrType>::getRand()
{
    return double(rand()) / 32768.0;
}




#endif
