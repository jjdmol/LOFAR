//# ConvertParset.h: Convert a ParSet file from SAS to cimager format
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <APS/ParameterSet.h>

namespace LOFAR {

  class MWImager
  {
  public:
    // Convert the input parset file (in SAS format) to the output parset
    // (in cimager format).
    // The output parset is written if the output name is not empty.
    static ACC::APS::ParameterSet convertParset (const std::string& nameIn,
						 const std::string& nameOut);

    // Convert the input parset (in SAS format) to the output parset
    // (in cimager format).
    // The output parset is written if the output name is not empty.
    static ACC::APS::ParameterSet convertParset (const ACC::APS::ParameterSet&,
						 const std::string& nameOut);
  };

}  //# namespace LOFAR
