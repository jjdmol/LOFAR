#include "TopLevelStrategy.h"
#include "ControllerFactory.h"
#include "directives/Parser.h"

#include "blackboard/debug.h"


//##ModelId=3F43290401D9
TopLevelStrategy * TopLevelStrategy::_instance = NULL;

//##ModelId=3F43262B0291
void TopLevelStrategy::run()
{
  TRACE t("TopLevelStrategy::run()");
}

std::vector<IntermediateLevelStrategy> &TopLevelStrategy::makeChildren(std::vector<Directive> &dirs)
{
  TRACE t("TopLevelStrategy::makeChildren(std::vector<Directive> &)");
  std::vector<IntermediateLevelStrategy> *vec = new std::vector<IntermediateLevelStrategy>();
  //iterate over directives
  for (std::vector<Directive>::iterator iter = dirs.begin() ;
       iter != dirs.end() ;
       ++iter)
  {
    vec->push_back(ControllerFactory::newStrategy(*iter));
  }
  // and make controllers
  return *vec;
}

void TopLevelStrategy::init()
{
  TRACE t("TopLevelStrategy::init()");
  intermediateStrategies = makeChildren(itsDirective.getParts());//new std::vector<IntermediateLevelStrategy>;
}

//##ModelId=3F4326D20214
TopLevelStrategy::TopLevelStrategy():itsDirective()
{
  TRACE t("TopLevelStrategy::TopLevelStrategy()");
  init();
}


//##ModelId=3F4E2F9A029F
TopLevelStrategy::~TopLevelStrategy()
{
}

//##ModelId=3F43262B0291
TopLevelStrategy &TopLevelStrategy::Instance()
{
  TRACE t("TopLevelStrategy::Instance()");
  if (_instance == 0)
  {
    _instance = new TopLevelStrategy();
  }
   return *_instance;
}
