
#ifndef TINYCEP_EXAMPLE_H
#define TINYCEP_EXAMPLE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Common/KeyValueMap.h>

#include <Transport/DataHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>
#include <tinyCEP/TinyDataManager.h>
#include <DH_Example.h>
#include <WH_Example.h>

namespace LOFAR
{
  class MyExample: public TinyApplicationHolder
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
