#ifndef SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530
#define SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530
    
#include <Solver/BaseSolver.h>

//##ModelId=3E00ACBD03DD
class DummySolver : public BaseSolver
{
  public:
    //##ModelId=3E00ACF002AB
    DummySolver(VisInputAgent &in, VisOutputAgent &out, SolverControlAgent &control);

    //##ModelId=3E00ACED00BB
    virtual void run();

};

#endif /* SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530 */
