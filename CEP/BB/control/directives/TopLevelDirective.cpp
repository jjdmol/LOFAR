#include "TopLevelDirective.h"

#include "blackboard/debug.h"

//const std::string TopLevelDirective::scriptID("start");

TopLevelDirective::TopLevelDirective(const std::string & initID):Directive(initID),scriptID(initID)
{
  TRACE t("TopLevelDirective::TopLevelDirective(" + initID + ")");
}
