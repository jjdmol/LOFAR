#ifndef TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372
#define TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372
#include "IntermediateLevelStrategy.h"
#include "Strategy.h"
#include "MPIProgramEntry.h"

//##ModelId=3F3A4C45002E
//##Documentation
//## The top level strategy is the top of the control hierarchy. It can exist
//## only once in a run.
class TopLevelStrategy : public Strategy, public MPIProgramEntry
{
  public:
    //##ModelId=3F4225A301D4
    run();
    //##ModelId=3F43262B0291
    static TopLevelStrategy &instance();


    //##ModelId=3F3A4C8201F4
    IntermediateLevelStrategy intermediateStrategies;
  protected:
    //##ModelId=3F4326D20214
    TopLevelStrategy();

  private:
    //##ModelId=3F43290401D9
    static TopLevelStrategy * instance;


};



#endif /* TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372 */
