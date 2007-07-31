//# BBSKernelStructs.h: some global structs used in the kernel.
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BBSKERNEL_BBSKERNELSTRUCTS_H
#define LOFAR_BBSKERNEL_BBSKERNELSTRUCTS_H

#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iosfwd.h>
#include <utility>

namespace LOFAR
{
  using std::pair;

  //# Forward declarations
  class BlobIStream;
  class BlobOStream;

  namespace BBS
  {
    class MeqDomain;

    // Information about which correlation products (auto, cross, or both),
    // and which polarizations should be used.
    struct Correlation
    {
      Correlation() : selection("NONE") {}
      string selection;     ///< Valid values: "NONE", "AUTO", "CROSS", "ALL"
      vector<string> type;  ///< E.g., ["XX", "XY", "YX", "YY"]
    };

    // Two vectors of stations names, which, when paired element-wise, define
    // the baselines to be used in the current step. Names may contain
    // wildcards, like \c * and \c ?. If they do, then all possible baselines
    // will be constructed from the expanded names. Expansion of wildcards
    // will be done in the BBS kernel.
    // 
    // For example, suppose that: 
    // \verbatim 
    // station1 = ["CS*", "RS1"]
    // station2 = ["CS*", "RS2"] 
    // \endverbatim
    // Furthermore, suppose that \c CS* expands to \c CS1, \c CS2, and \c
    // CS3. Then, in the BBS kernel, seven baselines will be constructed:
    // \verbatim
    // [ CS1:CS1, CS1:CS2, CS1:CS3, CS2:CS2, CS2:CS3, CS3:CS3, RS1:RS2 ]
    // \endverbatim
    // 
    // \note Station names are \e not expanded by matching with all existing
    // %LOFAR stations, but only with those that took part in a particular
    // observation; i.e., only those stations that are mentioned in the \c
    // ANTENNA table in the Measurement Set.
    struct Baselines
    {
      vector<string> station1;
      vector<string> station2;
    };


    struct Context
    {
      Baselines         baselines;
      Correlation       correlation;
      vector<string>    sources;
      vector<string>    instrumentModel;
    };


    struct PredictContext: Context
    {
      string            outputColumn;
    };


    struct SubtractContext: Context
    {
      string            outputColumn;
    };


    struct CorrectContext: Context
    {
      string            outputColumn;
    };


    struct GenerateContext: Context
    {
      vector<string>        unknowns;
      vector<string>        excludedUnknowns;
      pair<size_t, size_t>  domainSize;
    };

    // I/O stream methods
    // @{
    ostream& operator<<(ostream&, const Correlation&);
    ostream& operator<<(ostream&, const Baselines&);
    // @}

    // Blob I/O stream methods
    // @{
    BlobOStream& operator<<(BlobOStream&, const Correlation&);
    BlobOStream& operator<<(BlobOStream&, const Baselines&);

    BlobIStream& operator>>(BlobIStream&, Correlation&);
    BlobIStream& operator>>(BlobIStream&, Baselines&);
    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
