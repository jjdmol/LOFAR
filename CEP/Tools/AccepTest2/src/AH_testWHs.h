#ifndef AH_TESTWHS_H
#define AH_TESTWHS_H

#include <lofar_config.h>
#include <fstream>
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

namespace LOFAR
{

  using std::fstream;

  class AH_testWHs: public TinyApplicationHolder {
  public:
    AH_testWHs();
    ~AH_testWHs();

  protected:
    virtual void define(const KeyValueMap& kvm);
    virtual void undefine();
    virtual void init();
    virtual void run(int nsteps);
    virtual void quit();

    // Forbid copy constructor
    AH_testWHs (const AH_testWHs&);
    // Forbid assignment
    AH_testWHs& operator= (const AH_testWHs&);
    
    vector<WorkHolder*> itsWHs;

    void connectWHs(WorkHolder* srcWH, int srcDH, WorkHolder* dstWH, int dstDH);
  private:
    fstream* itsFileOutput;
  };
}

#endif
