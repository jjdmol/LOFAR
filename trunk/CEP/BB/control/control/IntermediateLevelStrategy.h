#ifndef INTERMEDIATELEVELSTRATEGY_H_HEADER_INCLUDED_C0C4CED7
#define INTERMEDIATELEVELSTRATEGY_H_HEADER_INCLUDED_C0C4CED7
#include "LowerStrategy.h"
//#include "directives/Directive.h"

//##ModelId=3F3A4C6C01E4
class IntermediateLevelStrategy : public LowerStrategy
{
 public:

  IntermediateLevelStrategy(const IntermediateLevelStrategy & other):LowerStrategy(other)
  {
  }

  IntermediateLevelStrategy():LowerStrategy()
  {
  }

  virtual ~IntermediateLevelStrategy(){}
 private:

};



#endif /* INTERMEDIATELEVELSTRATEGY_H_HEADER_INCLUDED_C0C4CED7 */
