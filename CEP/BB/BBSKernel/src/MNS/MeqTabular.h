//# MeqTabular.h: A tabular parameter value
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

#ifndef MNS_MEQTABULAR_H
#define MNS_MEQTABULAR_H

// \file
// A tabular (non-solvable) parameter value.

//# Includes
#include <BBSKernel/MNS/MeqFunklet.h>

namespace LOFAR
{
namespace BBS
{
 
// \ingroup BBSKernel
// \addtogroup MNS
// @{

  class MeqTabular: public MeqFunklet
  {
  public:
    // Create an empty object.
    MeqTabular()
      {}

    // Create a tabular from a ParmValue object.
    MeqTabular (const LOFAR::ParmDB::ParmValue&);

    // Convert a tabular to a ParmValue object.
    LOFAR::ParmDB::ParmValue toParmValue() const;

    virtual ~MeqTabular();

    // Clone the polc.
    virtual MeqTabular* clone() const;

    // Calculate the value.
    // Perturbations are not allowed.
    virtual MeqResult getResult (const MeqRequest&,
              int nrpert, int pertInx);
    virtual MeqResult getAnResult (const MeqRequest&,
          int nrpert, int pertInx);
  };

// @}

} // namespace BBS
} // namespace LOFAR

#endif
