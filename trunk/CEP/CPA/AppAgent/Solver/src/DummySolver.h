#ifndef SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530
#define SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530
    
#include <Solver/BaseSolver.h>
#include <AppUtils/src/ApplicationBase.h>
#include <SolverControl/src/SolverControlAgent.h>
#include <VisAgent/src/InputAgent.h>
#include <VisAgent/src/OutputAgent.h>

//##ModelId=3E00ACBD03DD
class DummySolver : public ApplicationBase
{
  public:
    //##ModelId=3E00ACF002AB
    DummySolver (VisAgent::InputAgent &in, VisAgent::OutputAgent &out, SolverControlAgent &ctrl);

    //##ModelId=3E00ACED00BB
    virtual void run (DataRecord::Ref& initrec);
    
    //##ModelId=3E00B22801E4
    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

  private:
    //##ModelId=3E63622A0380
    VisAgent::InputAgent &  input_;
    //##ModelId=3E63627000D3
    VisAgent::OutputAgent & output_;
    //##ModelId=3E6362BD011B
    SolverControlAgent &    control_;
    
    AppAgent::Ref input_ref;
    AppAgent::Ref output_ref;
    
  public:
    //##ModelId=3E00C7DC027D
    VisAgent::InputAgent &  input   ()
    { return input_; }
    //##ModelId=3E00C7DC0304
    VisAgent::OutputAgent & output  ()
    { return output_; }
    //##ModelId=3E00C7DC0393
    SolverControlAgent &    control ()
    { return control_; }


};

#endif /* SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530 */
