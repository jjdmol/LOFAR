//#  ApplicationHolder.cc: Base class for a user application in tinyCEP
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

#include <tinyCEP/ApplicationHolder.h>

namespace LOFAR
{
//   ApplicationHolder::ApplicationHolder(int ninput, int noutput, DataHolder* dhptr)
//     : itsArgc(0),
//       itsArgv(0),
//       itsProto(dhptr),
//       itsDataManager(0) {
//       itsNinputs(ninput),
//       itsNoutputs(noutput){
    
//     itsDataManager = new MiniDataManager(ninput, noutput);

//     for (int i=0; i < ninput; i++) {
//       itsDataManager->addInDataHolder(i, dhptr);
//     }
//     for (int i=0; i < noutput; i++){
//       itsDataManager->addOutDataHolder(i, dhptr);
//     }
//   }

  ApplicationHolder::ApplicationHolder()
    { }

  ApplicationHolder::~ApplicationHolder() {
    // dit segfault op dit moment nog.. FIXME!
    //    delete itsProto;
    //    delete itsDataManager;
  }

  ApplicationHolder::ApplicationHolder(const ApplicationHolder& that)
    : itsArgc        (that.itsArgc),
      itsArgv        (that.itsArgv),
      itsProto       (that.itsProto) { 
  }
      
    
  void ApplicationHolder::baseDefine(const KeyValueMap& params) {
    // Initialize MPI environment
    //    TRANSPORTER::init(itsArgc, itsArgv);

    // Let derived class define the Application
    define(params);
  }

  void ApplicationHolder::baseCheck() {
  }

  void ApplicationHolder::basePrerun() {
    //    itsDataManager->preprocess();
    init();
  }

  void ApplicationHolder::baseRun(int nsteps) {
    run(nsteps);
  }

  void ApplicationHolder::baseDump() {
    dump();
  }

  void ApplicationHolder::baseDHFile(const string& dh, const string& name) {
  }
  
  void ApplicationHolder::basePostrun() {
  }

  void ApplicationHolder::baseQuit() {
  }

  void ApplicationHolder::define(const KeyValueMap& map) {
  }
  
  void ApplicationHolder::init() {
  }
  
  void ApplicationHolder::run(int nsteps) {
  }
  
  void ApplicationHolder::run_once() {
  }

  void ApplicationHolder::quit() {
  }

  void ApplicationHolder::dump() const {
  }

  void ApplicationHolder::setarg (int argc, const char** argv) {
    itsArgc = argc;
    itsArgv = argv;
  }


  void ApplicationHolder::getarg (int* argc, const char** argv[]) {
  *argc = itsArgc;
  *argv = itsArgv;
  }



} // namespace LOFAR
