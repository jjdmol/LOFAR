#ifndef LOWERSTRATEGY_H_HEADER_INCLUDED_C0C4F7A2
#define LOWERSTRATEGY_H_HEADER_INCLUDED_C0C4F7A2
#include "Strategy.h"
class IntermediateLevelStrategy;

//##ModelId=3F3A57460290
class LowerStrategy : public Strategy
{
 public:
  void setParent(Strategy* p)
  {
    parent = p;
  }
  void init();
  virtual ~LowerStrategy(){}

 private:
  //##ModelId=3F3A4C820203
  Strategy *parent;

  std::vector<LowerStrategy> makeChildren(std::vector<Directive> &dirs);

  std::vector<LowerStrategy> children;
};



#endif /* LOWERSTRATEGY_H_HEADER_INCLUDED_C0C4F7A2 */
