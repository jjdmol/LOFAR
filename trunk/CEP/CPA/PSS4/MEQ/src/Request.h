//# Request.h: The request for an evaluation of an expression
//#
//# Copyright (C) 2002
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

#ifndef MEQ_REQUEST_H
#define MEQ_REQUEST_H

//# Includes
#include <MEQ/Cells.h>
#include <MEQ/Forest.h>
#include <DMI/DataRecord.h>


// This class represents a request for which an expression has to be
// evaluated. It contains the domain and cells to evaluate for.
// A flag tells if derivatives (perturbed values) have to be calculated.

namespace MEQ {

class Request : public DataRecord
{
public:
  // Create the request from the cells for which the expression has
  // to be calculated. Optionally no derivatives are calculated.
  explicit Request (const DataRecord&, Forest* forest=0,
		    bool calcDeriv=true, int id=0);

  // Create the request from the cells for which the expression has
  // to be calculated. Optionally no derivatives are calculated.
  explicit Request (const Cells&, Forest* forest=0,
		    bool calcDeriv=true, int id=0);

  // Calculate derivatives if parameters are solvable?
  bool calcDeriv() const
    { return itsCalcDeriv; }

  // Set new domain cells.
  void setCells (const Cells& cells)
    { itsCells = cells; }

  // Get the domain cells.
  const Cells& cells() const
    { return itsCells; }

  // Set the request id.
  void setId (int id)
    { itsId = id; }

  // Get the request id.
  int getId() const
    { return itsId; }

  // Set the forest.
  void setForest (Forest* forest)
    { itsForest = forest; }

  // Get the forest.
  Forest* getForest()
    { return itsForest; }

  // Get the rider subrecord.
  DataRecord::Ref& getRider();

private:
  int     itsId;
  bool    itsCalcDeriv;
  Cells   itsCells;
  Forest* itsForest;
};


} // namespace MEQ

#endif
