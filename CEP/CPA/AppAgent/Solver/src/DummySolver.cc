#include "DummySolver.h"

//##ModelId=3E00ACF002AB
DummySolver::DummySolver(VisInputAgent &in, VisOutputAgent &out, SolverControlAgent &control)
    : BaseSolver(in,out,control)
{
}



//##ModelId=3E00ACED00BB
void DummySolver::run ()
{
  using namespace SolverControl;
  
  cdebug(1)<< "starting run()\n";
  
  DataRecord::Ref params;
  // outer loop over time domains
  for( int dom = 0; dom < 3; dom++ )
  {
    control().startDomain();
    // second loop over solve jobs. Each call to startSoltion
    // returns a different set of params; once all sets have
    // been run through, the return code is NEXT_DOMAIN.
    while( control().startSolution(params) == CONTINUE )
    {
      cdebug(1)<< "startSolution: "<<params->sdebug(3)<<endl;
      cdebug(3)<< sdebug(3) <<endl;

      int niter = 0;
      double converge = 1000;
      // inner loop iterates solutions
      do
      {
        converge /= 10;
        niter++;
      }
      while( !control().endIteration(converge) );
      cdebug(1)<< "stopped after "<<niter<<" iterations, converge="<<converge<<endl;
      cdebug(3)<< sdebug(3) <<endl;
    }
    control().endDomain();
  }
  
  input().close();
  output().close();
  control().close();
  
  cdebug(1)<< "run() finished\n";
}

