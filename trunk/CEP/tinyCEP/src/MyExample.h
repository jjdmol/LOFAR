
#ifndef TINYCEP_EXAMPLE_H
#define TINYCEP_EXAMPLE_H

#include <lofar_config.h>

#include <Common/KeyValueMap.h>

#include <Transport/DataHolder.h>
#include <tinyCEP/ApplicationHolder.h>
#include <tinyCEP/BaseDataManager.h>
#include <tinyCEP/MiniDataManager.h>
#include <tinyCEP/DH_Example.h>
#include <tinyCEP/WH_Example.h>

namespace LOFAR
{
  class MyExample: public ApplicationHolder
  {
  public:
    MyExample(int ninput, int noutput);
    ~MyExample();

    void define(const KeyValueMap& map);
    void init();
    void run(int nsteps);
    void run_once();
    void quit();
    void dump() const;

    WH_Example* itsWorkHolder;
  private:
    int    itsArgv;
    char** itsArgc;

    DataHolder* itsProto;
    //    MiniDataManager* itsDataManager;

    DH_Example* dhptr;

    int itsNinputs;
    int itsNoutputs;
  };

} // namespace LOFAR
#endif
