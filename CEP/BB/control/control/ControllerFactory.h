#ifndef CONTROLLERFACTORY_H_HEADER_INCLUDED_C09100BC
#define CONTROLLERFACTORY_H_HEADER_INCLUDED_C09100BC
#include "Control.h"
#include "directives/DirectiveData.h"
#include "IntermediateLevelStrategy.h"
#include "LowerStrategy.h"

//##ModelId=3F6EC6F3007D
class ControllerFactory
{
  public:
    //##ModelId=3F6EC713008C
    static Control& newController(Directive & dir);

    //##ModelId=3F6EC77A02EE
    static IntermediateLevelStrategy& newStrategy(Directive & dir);

    //##ModelId=3F6EC7B2032C
    static LowerStrategy& newTactic(Directive & dir);
};



#endif /* CONTROLLERFACTORY_H_HEADER_INCLUDED_C09100BC */
