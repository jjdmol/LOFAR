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

#include <tinyCEP/TinyApplicationHolder.h>
#include TRANSPORTERINCLUDE

namespace LOFAR
{
//   TinyApplicationHolder::TinyApplicationHolder(int ninput, int noutput, DataHolder* dhptr)
//     : itsArgc(0),
//       itsArgv(0),
//       itsProto(dhptr),
//       itsDataManager(0) {
//       itsNinputs(ninput),
//       itsNoutputs(noutput){
//   }

  TinyApplicationHolder::TinyApplicationHolder()
    : itsArgc(0),
      itsArgv(0),
      itsProto(0)
    { }

  TinyApplicationHolder::~TinyApplicationHolder() {
    // dit segfault op dit moment nog.. FIXME!
    //    delete itsProto;
  }

  TinyApplicationHolder::TinyApplicationHolder(const TinyApplicationHolder& that)
    : itsArgc        (that.itsArgc),
      itsArgv        (that.itsArgv),
      itsProto       (that.itsProto) { 
  }
      
    
  void TinyApplicationHolder::baseDefine(const KeyValueMap& params) {
    // Initialize MPI environment
    TRANSPORTER::init(itsArgc, itsArgv);

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

  void TinyApplicationHolder::baseDHFile(const string& dh, const string& name) {
  }
  
  void TinyApplicationHolder::basePostrun() {
    postrun();
  }

  void TinyApplicationHolder::baseQuit() {
    quit();
  }

  void TinyApplicationHolder::define(const KeyValueMap& map) {
  }
  
  void TinyApplicationHolder::check() {
  }

  void TinyApplicationHolder::init() {
  }
  
  void TinyApplicationHolder::run(int nsteps) {
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



} // namespace LOFAR
