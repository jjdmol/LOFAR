#ifndef TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372
#define TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372
#include "IntermediateLevelStrategy.h"
#include "Strategy.h"
#include "directives/TopLevelDirective.h"
#include "blackboard/MPIProgramEntry.h"

#include <vector>

//##ModelId=3F3A4C45002E
//##Documentation
//## The top level strategy is the top of the control hierarchy. It can exist
//## only once in a run.
class TopLevelStrategy : public Strategy, public MPIProgramEntry
{
  public:
    //##ModelId=3F43262B0291
    void run();
    void init();
    //##ModelId=3F43262B0291
    static TopLevelStrategy &Instance();

    //##ModelId=3F4E2F9A0290
    std::vector<IntermediateLevelStrategy> intermediateStrategies;
  protected:
    //##ModelId=3F4326D20214
    TopLevelStrategy();
    //##ModelId=3F4E2F9A029F
    virtual ~TopLevelStrategy();

  private:
    //##ModelId=3F43290401D9
    static TopLevelStrategy * _instance;

    TopLevelDirective itsDirective;

    std::vector<IntermediateLevelStrategy> makeChildren(std::vector<Directive> &dirs);
};

#endif /* TOPLEVELSTRATEGY_H_HEADER_INCLUDED_C0C4B372 */
