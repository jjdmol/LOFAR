#ifndef LOWERSTRATEGY_H_HEADER_INCLUDED_C0C4F7A2
#define LOWERSTRATEGY_H_HEADER_INCLUDED_C0C4F7A2
#include "Strategy.h"
class IntermediateLevelStrategy;

//##ModelId=3F3A57460290
class LowerStrategy : public Strategy
{
  public:
    //##ModelId=3F3A4CA60232
    IntermediateLevelStrategy *parent;

};



#endif /* LOWERSTRATEGY_H_HEADER_INCLUDED_C0C4F7A2 */
