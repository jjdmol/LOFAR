
#ifndef TINYCEP_EXAMPLE_H
#define TINYCEP_EXAMPLE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Blob/KeyValueMap.h>

#include <Transport/DataHolder.h>
#include <Transport/TH_Mem.h>

#include <tinyCEP/TinyApplicationHolder.h>
#include <tinyCEP/TinyDataManager.h>

#include <DH_Example.h>
#include <WH_Example.h>

namespace LOFAR
{
  class MySecondExample: public TinyApplicationHolder
  {
  public:
    MySecondExample(int ninput, int noutput);
    ~MySecondExample();

    void define(const KeyValueMap& map);
    void init();
    void run(int nsteps);
    void run_once();
    void quit();
    void dump() const;

    //    WH_Example* itsWorkHolder;
    WorkHolder* itsWHs[2048];
    
  private:
    int    itsArgv;
    char** itsArgc;

    Connection* itsConn;
    TransportHolder* itsTH;

    DH_Example* dhptr;

    int itsNinputs;
    int itsNoutputs;

    int itsWHCount;
  };

} // namespace LOFAR
#endif
