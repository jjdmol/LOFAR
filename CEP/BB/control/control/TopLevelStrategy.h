#ifndef TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372
#define TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372
#include "IntermediateLevelStrategy.h"
#include "Strategy.h"
#include "MPIProgramEntry.h"

//##ModelId=3F3A4C45002E
class TopLevelStrategy : public Strategy, public MPIProgramEntry
{
  public:
    //##ModelId=3F4225A301D4
    run();

    //##ModelId=3F3A4C8201F4
    IntermediateLevelStrategy intermediateStrategies;

};



#endif /* TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372 */
