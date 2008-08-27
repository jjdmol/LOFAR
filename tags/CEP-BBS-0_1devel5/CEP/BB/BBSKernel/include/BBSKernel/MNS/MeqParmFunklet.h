//# MeqParmFunklet.h: Stored parameter with polynomial coefficients
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

#ifndef MNS_MEQPARMFUNKLET_H
#define MNS_MEQPARMFUNKLET_H

// \file
// A stored parameter with funklet coefficients

//# Includes
#include <BBSKernel/MNS/MeqParm.h>
#include <BBSKernel/MNS/MeqFunklet.h>
#include <ParmDB/ParmDB.h>
#include <Common/lofar_vector.h>

// Forward Declarations
//namespace LOFAR {
//  class ParmData;
//}

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

// This class contains the coefficients of a 2-dim funklet.
// The order in frequency and time must be given.
// The nr of coefficients is (1+order(freq)) * (1+order(time)).
// The coefficients are numbered 0..N with the frequency as the most rapidly
// varying axis.

class MeqParmFunklet: public MeqParm
{
public:
  // Create a stored paramater with the given name and type.
  MeqParmFunklet (const string& name, LOFAR::ParmDB::ParmDB* table);

  virtual ~MeqParmFunklet();

  // Make the correct object depending on the parm type.
  // For a parm expression a MeqParmExpr object is created, otherwise
  // a MeqParmFunklet.
  static MeqExpr create (const string& name,
			 MeqParmGroup& group,
			 LOFAR::ParmDB::ParmDB* table);

  // Get the requested result of the parameter.
  virtual MeqResult getResult (const MeqRequest&);

  // Remove all funklets.
  virtual void removeFunklets();

  // Fill the funklets from the ParmValues for the given work domain.
  // Only fill if not filled yet.
  // It means that clearFunklets need to be called if a new work domain
  // is to be processed.
  virtual void fillFunklets (const std::map<std::string,LOFAR::ParmDB::ParmValueSet>&,
			     const MeqDomain&);

  // Add a funklet.
  void add (const MeqFunklet& funklet);

  virtual int setSolvable(int spid);
  virtual void unsetSolvable();

  // Initialize the solvable parameter for the given solve domain size.
  // FillFunklets must have been done before.
  // The only important thing about the solve domain is its size, not the
  // start and end values. However, it is checked if the start of the
  // solve domain matches the start of the work domain given to fillFunklets.
  // It returns the number of spids used for this parm.
  virtual void initDomain (const vector<MeqDomain>&);

  // Get the current funklets.
  virtual const vector<MeqFunklet*>& getFunklets() const;

  // Get the ParmDBInfo
  virtual LOFAR::ParmDB::ParmDBMeta getParmDBMeta() const;

  // Get the ParmDB seqnr.
  virtual int getParmDBSeqNr() const;

  // Make the new value persistent (for the given domain).
  virtual void save();
  virtual void save(size_t domainIndex);

  // Update the solvable parameter with the new value.
//  virtual void update(const ParmData& values);

  // Update the solvable parameter coefficients with the new values.
  // The vector contains all solvable values; it picks out the values
  // at the spid index of this parameter.
  virtual void update(const vector<double>& value);
  virtual void update(size_t cell, const vector<double> &coeff);
  virtual void update(size_t cell, const vector<double> &coeff, size_t offset);

  // Update the solvable parameter coefficients with the new values
  // in the table.
  // The default implementation throws a "not implemented" exception.
  virtual void updateFromTable();

  // Get the perturbation index.
  int getPertInx() const
    { return itsPertInx; }


private:
  bool                itsDefUsed;  //# true = default is used as initial value
  int                 itsNrPert;   //# Nr of perturbed values for this parm
                                   //# This is the max nr of scids/funklet
  int                 itsPertInx;  //# index of first perturbed value in result
  vector<MeqFunklet*> itsFunklets;
  LOFAR::ParmDB::ParmDB*     itsTable;
  MeqDomain           itsWorkDomain;
};


// @}

} // namespace BBS
} // namespace LOFAR

#endif
