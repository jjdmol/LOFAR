#include "ControllerFactory.h"
#include "Driver.h"

//##ModelId=3F6EC713008C
Control & ControllerFactory::newController(Directive & dir)
{
  // this should be an intelligent check for the right symbol
  Control *objp(new Driver());
  objp->addDirective(&dir);
  return *objp;
}

//##ModelId=3F6EC77A02EE
IntermediateLevelStrategy & ControllerFactory::newStrategy(Directive & dir)
{
  IntermediateLevelStrategy *objp(new IntermediateLevelStrategy());
  objp->addDirective(&dir);
  return *objp;
}

//##ModelId=3F6EC7B2032C
LowerStrategy & ControllerFactory::newTactic(Directive & dir)
{
  LowerStrategy *objp(new LowerStrategy());
  objp->addDirective(&dir);
  return *objp;
}

