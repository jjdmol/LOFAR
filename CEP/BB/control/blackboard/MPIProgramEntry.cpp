#include "MPIProgramEntry.h"
#include "debug.h"

void MPIProgramEntry::run()
{
  DEBUG(std::string(typeid(*this).name()) + " is running");
}
