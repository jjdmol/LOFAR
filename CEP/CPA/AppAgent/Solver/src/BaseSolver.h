#ifndef SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B
#define SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B
    
#include <VisAgent/VisInputAgent.h>
#include <VisAgent/VisOutputAgent.h>
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
    BaseSolver (VisInputAgent &in,VisOutputAgent &out,
                SolverControlAgent &control);

    //##ModelId=3E00AA510160
    virtual bool init (const DataRecord::Ref &data);

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
    VisInputAgent & inputAgent;

    //##ModelId=3E00AA51012C
    VisOutputAgent & outputAgent;

    //##ModelId=3E00AA51013D
    SolverControl::SolverControlAgent & controlAgent;
    
  protected:
      
    //##ModelId=3E00C7DC027D
    VisInputAgent  & input  ()      { return inputAgent; }
  
    //##ModelId=3E00C7DC0304
    VisOutputAgent & output ()      { return outputAgent; }
    
    //##ModelId=3E00C7DC0393
    SolverControl::SolverControlAgent & control () { return controlAgent; }
      
};



#endif /* SOLVER_SRC_BASESOLVER_H_HEADER_INCLUDED_F6597D9B */
