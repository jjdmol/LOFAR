#include "TopLevelStrategy.h"

//##ModelId=3F4225A301D4
//void TopLevelStrategy::run()
//{
//}

//##ModelId=3F43290401D9
TopLevelStrategy * TopLevelStrategy::_instance = NULL;

//##ModelId=3F43262B0291
void TopLevelStrategy::run()
{
}

//##ModelId=3F4326D20214
TopLevelStrategy::TopLevelStrategy()
{
  intermediateStrategies = new std::vector<IntermediateLevelStrategy>;
}


//##ModelId=3F4E2F9A029F
TopLevelStrategy::~TopLevelStrategy()
{
  delete intermediateStrategies;
}

//##ModelId=3F43262B0291
TopLevelStrategy &TopLevelStrategy::Instance()
{
   if (_instance == 0) {
     _instance = new TopLevelStrategy();
   }
   return *_instance;
}
