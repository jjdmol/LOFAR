#ifndef SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B
#define SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B
    
#include <VisAgent/InputAgent.h>
#include <VisAgent/OutputAgent.h>
#include <SolverControl/SolverControlAgent.h>
#include <Solver/AID-Solver.h>
    
#pragma aidgroup Solver
#pragma aid Domain Parameter Step Convergence
    
using SolverControl::SolverControlAgent;

//##ModelId=3E00AA510095
class BaseSolver
{
  public:
    //##ModelId=3E00AC30030A
    BaseSolver (VisAgent::InputAgent &in,VisAgent::OutputAgent &out,
                SolverControlAgent &control);

    //##ModelId=3E00AA510160
    virtual bool init (const DataRecord &data);

    //##ModelId=3E00AA510173
    virtual void run () = 0;
    
    
    //##ModelId=3E00B22801E4
    virtual string sdebug ( int detail = 1,const string &prefix = "",
                            const char *name = 0 ) const;
    
    //##ModelId=3E00B22A011F
    const char * debug ( int detail = 1,const string &prefix = "",
                         const char *name = 0 ) const
    { return Debug::staticBuffer(sdebug(detail,prefix,name)); }
    
    
    //##ModelId=3E00C7DB0324
    LocalDebugContext;
    
  private:
    //##ModelId=3E00AA51011E
    VisAgent::InputAgent & inputAgent;

    //##ModelId=3E00AA51012C
    VisAgent::OutputAgent & outputAgent;

    //##ModelId=3E00AA51013D
    SolverControlAgent & controlAgent;
    
  protected:
      
    //##ModelId=3E00C7DC027D
    VisAgent::InputAgent  & input  ()      { return inputAgent; }
  
    //##ModelId=3E00C7DC0304
    VisAgent::OutputAgent & output ()      { return outputAgent; }
    
    //##ModelId=3E00C7DC0393
    SolverControlAgent    & control () { return controlAgent; }
      
};



#endif /* SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B */
