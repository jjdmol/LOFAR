#include "LowerStrategy.h"
#include "IntermediateLevelStrategy.h"
#include "ControllerFactory.h"

#include "blackboard/debug.h"


std::vector<LowerStrategy> LowerStrategy::makeChildren(std::vector<Directive> &dirs)
{
  TRACE t("LowerStrategy::makeChildren(std::vector<Directive> &)");
  std::vector<LowerStrategy> vec;// = new std::vector<LowerStrategy>();
  //iterate over directives
  for (std::vector<Directive>::iterator iter = dirs.begin() ;
       iter != dirs.end() ;
       ++iter)
  {
    TRACE pb("adding Tactic with directive: " + iter->getId());
    vec.push_back(ControllerFactory::newTactic(*iter));
  }
  // and make controllers
  return vec;
}

void LowerStrategy::init()
{
  TRACE t("LowerStrategy::init()");
  std::vector<Directive> dirs(itsDirective.getParts());
  children = makeChildren(dirs);//new std::vector<IntermediateLevelStrategy>;
  std::vector<LowerStrategy>::iterator i = children.begin();
  while(i != children.end())
  {
    i->setParent(this);
    i->init();
    ++i;
  } // NOTE: this one is recursively
}
