#include "TopLevelStrategy.h"

//##ModelId=3F4225A301D4
TopLevelStrategy::run()
{
}

//##ModelId=3F43290401D9
TopLevelStrategy * TopLevelStrategy::_instance = NULL;

//##ModelId=3F43262B0291
TopLevelStrategy &TopLevelStrategy::Instance()
{
}

//##ModelId=3F4326D20214
TopLevelStrategy::TopLevelStrategy()
{
   if (_instance == 0) {
      _instance = new Singleton;
   }
   return _instance;
}


