#ifndef CONTROLLERFACTORY_H_HEADER_INCLUDED_C09100BC
#define CONTROLLERFACTORY_H_HEADER_INCLUDED_C09100BC
#include "Control.h"
#include "DirectiveData.h"
#include "IntermediateLevelStrategy.h"
#include "LowerStrategy.h"

//##ModelId=3F6EC6F3007D
class ControllerFactory
{
  public:
    //##ModelId=3F6EC713008C
    Control newController(DirectiveData data);

    //##ModelId=3F6EC77A02EE
    IntermediateLevelStrategy newStrategy();

    //##ModelId=3F6EC7B2032C
    LowerStrategy newTactic(DirectiveData data);

};



#endif /* CONTROLLERFACTORY_H_HEADER_INCLUDED_C09100BC */
