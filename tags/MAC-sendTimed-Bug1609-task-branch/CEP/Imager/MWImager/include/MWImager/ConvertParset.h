//# ConvertParset.h: Convert a ParSet file from SAS to cimager format
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_MWIMAGER_CONVERTPARSET_H
#define LOFAR_MWIMAGER_CONVERTPARSET_H

// @file
// @brief Convert a ParSet file from SAS to cimager format
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWStep.h>

namespace LOFAR {

  // @ingroup MWCommon
  // @brief Convert a ParSet file from SAS to cimager format

  // This class contains a few static functions to convert a parset file
  // from the LOFAR SAS conventions to the cimager format.

  class MWImager
  {
  public:
    // Convert the input parset file (in SAS format) to the output parset
    // (in cimager format).
    // The output parset is written if the output name is not empty.
    static ParameterSet convertParset (const std::string& nameIn,
						 const std::string& nameOut);

    // Convert the input parset (in SAS format) to the output parset
    // (in cimager format).
    // The output parset is written if the output name is not empty.
    static ParameterSet convertParset (const ParameterSet&,
						 const std::string& nameOut);

    // Append the parameters of in to the output.
    // They are prefixed with the given prefix.
    // Parameter names in the <src>old2newNameMap</src> are renamed.
    // Parameters defined in the <src>defaults</src> are written in the output
    // if not defined in the input.
    static void convert (ParameterSet& out,
			 const ParameterSet& in,
			 const std::map<std::string,std::string>& old2NewNameMap,
			 const std::map<std::string,std::string>& defaults,
			 const std::string& prefix);
  };

}  //# end namespace LOFAR

#endif
