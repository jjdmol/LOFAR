//# AH_AccepTest.h: ApplicationHolder for acceptance test 2
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef AH_ACCEPTEST_H
#define AH_ACCEPTEST_H

#include <lofar_config.h>
#include <fstream>
#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/TinyApplicationHolder.h>

namespace LOFAR
{
  using std::fstream;

  class AH_AccepTest: public TinyApplicationHolder {
  public:
    AH_AccepTest();
    ~AH_AccepTest();

  protected:
    void define(const KeyValueMap& kvm);
    void undefine();
    void init();
    void run(int nsteps);
    void quit();

    // Forbid copy constructor
    AH_AccepTest (const AH_AccepTest&);
    // Forbid assignment
    AH_AccepTest& operator= (const AH_AccepTest&);
    
    vector<WorkHolder*> itsWHs;
    void connectWHs(WorkHolder* srcWH, int srcDH, WorkHolder* dstWH, int dstDH);
  private:
    fstream* itsFileOutput;
  };
}

#endif
