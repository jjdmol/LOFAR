#ifndef INTERMEDIATELEVELSTRATEGY_H_HEADER_INCLUDED_C0C4CED7
#define INTERMEDIATELEVELSTRATEGY_H_HEADER_INCLUDED_C0C4CED7
#include "LowerStrategy.h"
class TopLevelStrategy;

//##ModelId=3F3A4C6C01E4
class IntermediateLevelStrategy : public LowerStrategy
{
  public:
    //##ModelId=3F3A4C820203
    TopLevelStrategy *parent;

    //##ModelId=3F3A4CA60222
    LowerStrategy children;

};



#endif /* INTERMEDIATELEVELSTRATEGY_H_HEADER_INCLUDED_C0C4CED7 */
