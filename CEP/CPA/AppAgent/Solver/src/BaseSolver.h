#ifndef SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B
#define SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B
    
#include <Solver/SolverControlAgent.h>
#include <Solver/AID-Solver.h>
#include <AppUtils/VisPipe.h>
    
class AppControlAgent;
    
#pragma aidgroup Solver
#pragma aid Domain Parameter Step Convergence Size
    
//##ModelId=3E00AA510095
class BaseSolver : public VisPipe
{
  private:
    //##ModelId=3E6F604203D5
    SolverControl::SolverControlAgent * control_;
    
    
  public:
    //##ModelId=3E00AC30030A
    BaseSolver ();

    //##ModelId=3E7737FA01A9
    virtual void attach(AppControlAgent *ctrl, int flags);
    
    //##ModelId=3E6F60450088
    SolverControl::SolverControlAgent & control()   { return *control_; }
    //##ModelId=3E6F6045013F
    const SolverControl::SolverControlAgent & control() const   { return *control_; }


    //##ModelId=3E6F6043036E
    virtual string sdebug ( int detail = 1,const string &prefix = "",
                            const char *name = 0 ) const;
    //##ModelId=3E00C7DB0324
    LocalDebugContext;
    
};



#endif /* SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B */
