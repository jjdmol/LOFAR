#include <lofar_config.h>
#include <BBSControl/SenderId.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_set.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

int main()
{
  INIT_LOGGER("tSenderId");
  try {
    {
      SenderId id;
      cout << id << endl;
      ASSERTSTR(!id.isValid(), id);
    }
    {
      SenderId id(SenderId::KERNEL, 1);
      cout << id << endl;
      ASSERTSTR(id.isValid(), id);
    }
    {
      SenderId id(static_cast<SenderId::Type>(3), 5);
      cout << id << endl;
      ASSERTSTR(!id.isValid(), id);
    }
    {
      SenderId id(SenderId::SOLVER, -10);
      cout << id << endl;
      ASSERTSTR(!id.isValid(), id);
    }
    set<SenderId> idSet;
    ASSERT(idSet.insert(SenderId(SenderId::KERNEL, 1)).second);
    ASSERT(idSet.insert(SenderId(SenderId::KERNEL, 2)).second);
    ASSERT(idSet.insert(SenderId(SenderId::SOLVER, 1)).second);
    ASSERT(idSet.insert(SenderId(SenderId::SOLVER, 2)).second);
    ASSERT(!idSet.insert(SenderId()).second);
    ASSERT(!idSet.insert(SenderId(static_cast<SenderId::Type>(3), 5)).second);
    for (set<SenderId>::const_iterator it = idSet.begin(); 
         it != idSet.end(); ++it) {
      cout << *it << endl;
    }
  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  return 0;
}
