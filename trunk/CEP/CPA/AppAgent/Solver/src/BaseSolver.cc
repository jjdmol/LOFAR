#include "BaseSolver.h"

InitDebugContext(BaseSolver,"Solver");

using namespace SolverControl;

static int dum = aidRegistry_Solver();
        
BaseSolver::BaseSolver ()
    : VisPipe(),control_(0)
{
}

//##ModelId=3E7737FA01A9
void BaseSolver::attach (AppControlAgent *ctrl, int flags)
{
  AppAgent::Ref ref(ctrl,flags);
  FailWhen(control_,"solver control agent already attached");
  SolverControlAgent *sctrl = dynamic_cast<SolverControlAgent*>(ctrl);
  FailWhen(!sctrl,"specified agent is not a SolverControl");
  ApplicationBase::attach(control_ = sctrl,flags);
}

//##ModelId=3E6F6043036E
string BaseSolver::sdebug ( int detail,const string &prefix,const char *name ) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x %d",name?name:"Solver",(int)this,state());
  }
  if( detail >= 2 || detail == -2 )
  {
    out += "\n";
    out += prefix + "  Input:   " + input().sdebug(abs(detail)-1,prefix+"  ") + "\n";
    out += prefix + "  Output:  " + output().sdebug(abs(detail)-1,prefix+"  ") + "\n";
    out += prefix + "  Control: " + control().sdebug(abs(detail)-1,prefix+"  ");
  }
  return out;
}

