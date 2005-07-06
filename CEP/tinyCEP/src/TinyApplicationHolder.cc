///#  TinyApplicationHolder.cc: Base class for a user application in tinyCEP
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#include <Transport/BaseSim.h>
#include <tinyCEP/TinyApplicationHolder.h>
#include TRANSPORTERINCLUDE

namespace LOFAR
{

  TinyApplicationHolder::TinyApplicationHolder()
    : itsArgc(0),
      itsArgv(0)
    { }

  TinyApplicationHolder::~TinyApplicationHolder() {
  }

  TinyApplicationHolder::TinyApplicationHolder(const TinyApplicationHolder& that)
    : itsArgc        (that.itsArgc),
      itsArgv        (that.itsArgv)
  { 
  }
      
    
  void TinyApplicationHolder::baseDefine(const KeyValueMap& params) {
#ifdef HAVE_MPI
    // Initialize MPI environment
    TRANSPORTER::initMPI(itsArgc, itsArgv);
#endif
    // Let derived class define the Application
    define(params);
  }

  void TinyApplicationHolder::baseCheck() {
    check();
  }

  void TinyApplicationHolder::basePrerun() {
    init();
  }

  void TinyApplicationHolder::baseRun(int nsteps) {
    run(nsteps);
  }

  void TinyApplicationHolder::baseDump() {
    dump();
  }

  void TinyApplicationHolder::baseDHFile(const string&, const string&) {
  }
  
  void TinyApplicationHolder::basePostrun() {
    postrun();
  }

  void TinyApplicationHolder::baseQuit() {
    quit();
  }

  void TinyApplicationHolder::define(const KeyValueMap&) {
  }
  
  void TinyApplicationHolder::check() {
  }

  void TinyApplicationHolder::init() {
  }
  
  void TinyApplicationHolder::run(int) {
  }
  
  void TinyApplicationHolder::run_once() {
  }

  void TinyApplicationHolder::postrun() {
  }

  void TinyApplicationHolder::quit() {
  }

  void TinyApplicationHolder::dump() const {
  }

  void TinyApplicationHolder::setarg (int argc, const char** argv) {
    itsArgc = argc;
    itsArgv = argv;
  }


  void TinyApplicationHolder::getarg (int* argc, const char** argv[]) {
    *argc = itsArgc;
    *argv = itsArgv;
  }

  void TinyApplicationHolder::setParameters (const ACC::APS::ParameterSet& params) {
    itsParamSet = params;
  }

} // namespace LOFAR
