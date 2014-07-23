//# ParmDBBlob.cc: Dummy class to hold parmaeter values
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <ParmDB/ParmDBBlob.h>
#include <Common/LofarLogger.h>


using namespace std;

namespace LOFAR {
namespace BBS {

  ParmDBBlob::ParmDBBlob (const string&, bool)
  {}

  ParmDBBlob::~ParmDBBlob()
  {}

  void ParmDBBlob::flush (bool)
  {}

  void ParmDBBlob::lock (bool)
  {}

  void ParmDBBlob::unlock()
  {}

  void ParmDBBlob::clearTables()
  {}

  void ParmDBBlob::setDefaultSteps (const vector<double>&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  int ParmDBBlob::getNameId (const std::string&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  Box ParmDBBlob::getRange (const string&) const
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  Box ParmDBBlob::getRange (const std::vector<std::string>&) const
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  void ParmDBBlob::getValues (vector<ParmValueSet>&,
                              const vector<uint>&,
                              const vector<ParmId>&,
                              const Box&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  void ParmDBBlob::getDefValues (ParmMap&,
                                 const string&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  void ParmDBBlob::putValues (const string&, int&, ParmValueSet&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  void ParmDBBlob::putDefValue (const string&, const ParmValueSet&,
                                bool)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  void ParmDBBlob::deleteValues (const string&,
                                 const Box&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  void ParmDBBlob::deleteDefValues (const string&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  vector<string> ParmDBBlob::getNames (const string&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

  void ParmDBBlob::fillDefMap (ParmMap&)
  {
    THROW (Exception, "ParmDBBlob not implemented");
  }

} // namespace BBS
} // namespace LOFAR
