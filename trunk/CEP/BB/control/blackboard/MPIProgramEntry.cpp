#include "MPIProgramEntry.h"
#include "debug.h"

void MPIProgramEntry::run()
{
  DEBUG(typeid(*this).name() << " is running");
}
