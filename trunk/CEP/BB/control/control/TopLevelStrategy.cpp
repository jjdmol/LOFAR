#include "TopLevelStrategy.h"
#include "ControllerFactory.h"
#include "directives/Parser.h"

#include "blackboard/debug.h"

#include <sstream>

//##ModelId=3F43290401D9
TopLevelStrategy * TopLevelStrategy::_instance = NULL;

//##ModelId=3F43262B0291
void TopLevelStrategy::run()
{
  TRACE t("TopLevelStrategy::run()");
}

std::vector<IntermediateLevelStrategy> TopLevelStrategy::makeChildren(std::vector<Directive> &dirs)
{
  TRACE t("TopLevelStrategy::makeChildren(std::vector<Directive> &)");
  std::vector<IntermediateLevelStrategy> vec;// = new std::vector<IntermediateLevelStrategy>();
  //iterate over directives
  for (std::vector<Directive>::iterator iter = dirs.begin() ;
       iter != dirs.end() ;
       ++iter)
  {
    TRACE pb("adding Strategy with directive: " + iter->getId());
    IntermediateLevelStrategy newPart(ControllerFactory::newStrategy(*iter));
    vec.push_back(newPart);
  }
  // and make controllers
  return vec;
}

void TopLevelStrategy::init()
{
  TRACE t("TopLevelStrategy::init()");
  std::vector<Directive> dirs(itsDirective.getParts());
  DEBUG("parts called");

  std::ostringstream os;
  os << "number of parts: " << dirs.size();
  DEBUG(os.str());

  intermediateStrategies = makeChildren(dirs);//new std::vector<IntermediateLevelStrategy>;
  // now parse all the children;
  std::vector<IntermediateLevelStrategy>::iterator i = intermediateStrategies.begin();
  while(i != intermediateStrategies.end())
  {
    i->setParent(this);
    i->init();
    ++i;
  } // NOTE: this one is not recursive
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
