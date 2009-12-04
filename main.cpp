#include <time.h>

#include "Annealer.h"
#include "LocalOpt.h"
#include "TestHarness.h"
#include "TSPMoveMgr.h"



int
main(int   argc,
     char* argv[])
{
    clock_t start = clock();

    //TestHarnessMoveMgr thmm(1000);
    //Annealer<Move, int>	lo;
    //lo.optimize(&thmm);

    TSPMoveMgr tspmm(argv[1]);
    Annealer<TSPMove, double> sa;
    sa.optimize(&tspmm);
    
    tspmm.debug();

    std::cout << "Elapsed time = " << (float(clock() - start) / CLOCKS_PER_SEC) << "\n";

    return 0;
}
