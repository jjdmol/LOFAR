#ifndef SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530
#define SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530
    
#include <Solver/BaseSolver.h>

namespace SolverVocabulary
{
  using namespace ApplicationVocabulary;
  
  const HIID  FDomainSize         = AidDomain|AidSize;
  
  const HIID  InputInitFailed     = AidInput|AidInit|AidFail,
              OutputInitFailed    = AidOutput|AidInit|AidFail;
};
    
//##ModelId=3E00ACBD03DD
class DummySolver : public BaseSolver
{
  public:
    //##ModelId=3E00ACED00BB
    virtual void run ();
    
//    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

  private:
    //##ModelId=3E6F603A01D6
    void addTileToDomain (VisTile::Ref &tileref);
    //##ModelId=3E6F603A0256
    void checkInputState (int instat);
    //##ModelId=3E6F603A02D1
    void endDomain ();
    //##ModelId=3E6F603A030B
    void endSolution (const DataRecord &endrec);
      
    //##ModelId=3E6F60390007
    double domain_start;
    //##ModelId=3E6F6039007B
    double domain_end;
    //##ModelId=3E6F603900E3
    double domain_size;
    //##ModelId=3E6F60390147
    bool in_domain;
    
    //##ModelId=3E77394100CA
    int ntiles;
};

#endif /* SOLVER_SRC_DUMMYSOLVER_H_HEADER_INCLUDED_D8FEA530 */
