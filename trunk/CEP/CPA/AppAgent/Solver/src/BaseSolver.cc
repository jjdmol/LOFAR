#include "BaseSolver.h"

InitDebugContext(BaseSolver,"Solver");

static int dum = aidRegistry_Solver();
        
BaseSolver::BaseSolver (VisAgent::InputAgent &in,VisAgent::OutputAgent &out,
                        SolverControlAgent &control)
    : inputAgent(in),outputAgent(out),controlAgent(control)
{
}

bool BaseSolver::init (const DataRecord &data)
{
  return  inputAgent.init(data) && 
          outputAgent.init(data) && 
          controlAgent.init(data);
}


string BaseSolver::sdebug ( int detail,const string &prefix,const char *name ) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"BaseSolver",(int)this);
  }
  if( detail >= 2 || detail == -2 )
  {
    out += "\n";
    out += prefix + "  Input:   " + inputAgent.sdebug(abs(detail)-1,prefix+"  ") + "\n";
    out += prefix + "  Output:  " + outputAgent.sdebug(abs(detail)-1,prefix+"  ") + "\n";
    out += prefix + "  Control: " + controlAgent.sdebug(abs(detail)-1,prefix+"  ");
  }
  return out;
}
