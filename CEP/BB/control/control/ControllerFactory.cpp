#include "ControllerFactory.h"
#include "Driver.h"

#include "blackboard/debug.h"

//##ModelId=3F6EC713008C
Control & ControllerFactory::newController(Directive & dir)
{
  TRACE t("ControllerFactory::newController(Directive &)");
  // this should be an intelligent check for the right symbol
  Control *objp(new Driver());
  objp->addDirective(dir);
  return *objp;
}

//##ModelId=3F6EC77A02EE
IntermediateLevelStrategy & ControllerFactory::newStrategy(Directive & dir)
{
  TRACE t("ControllerFactory::newStrategy(Directive & " + dir.getId() + ")");
  IntermediateLevelStrategy *objp(new IntermediateLevelStrategy());
  objp->addDirective(dir);
  return *objp;
}

//##ModelId=3F6EC7B2032C
LowerStrategy & ControllerFactory::newTactic(Directive & dir)
{
  TRACE t("ControllerFactory::newTactic(Directive & " + dir.getId() + ")");
  LowerStrategy *objp(new LowerStrategy());
  objp->addDirective(dir);
  return *objp;
}

