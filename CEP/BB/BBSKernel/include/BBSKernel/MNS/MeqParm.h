//# MeqParm.h: The base class for a parameter
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

#ifndef MNS_MEQPARM_H
#define MNS_MEQPARM_H

// \file
// The base class for a parameter

//# Includes
#include <BBSKernel/MNS/MeqExpr.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <ParmDB/ParmValue.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

//# Forward declarations
class MeqDomain;
class MeqParmGroup;
class MeqFunklet;
class ParmData;

// This class is the (abstract) base class for parameters.
// The constructor assigns a unique id to the parameter and adds
// it to a map to find the id from the name.

class MeqParm : public MeqExprRep
{
public:
  // Create a parameter with the given name.
  // Assign a parameter id (a sequence number) to it.
  explicit MeqParm (const string& name);

  virtual ~MeqParm();

  // Get the parameter name.
  const string& getName() const
    { return itsName; }
  // Set the parameter name.
  void setName (const string& name)
    { itsName = name; }

  // Get the ParmDB seqnr.
  virtual int getParmDBSeqNr() const;

  // Remove all funklets.
  virtual void removeFunklets();

  // Read the polcs for the given domain.
  virtual void fillFunklets (const std::map<std::string,LOFAR::ParmDB::ParmValueSet>&,
                 const MeqDomain&);

  // Initialize the solvable parameter for the given domain.
  virtual int initDomain (const vector<MeqDomain>&, int& pertIndex,
              vector<int>& scidIndex);

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is the index of the first spid of this parm.
  // It returns the number of spids in this parm.
  void setSolvable (bool solvable)
    { itsIsSolvable = solvable; }

  bool isSolvable() const
    { return itsIsSolvable; }

  // Get the result of the parameter for the given domain.
  virtual MeqResult getResult (const MeqRequest&) = 0;

  // Get the current coefficients of the parameter.
  virtual const vector<MeqFunklet*>& getFunklets() const;

  // Make the new value persistent (for the given domain).
  virtual void save();
  virtual void save(size_t domainIndex);

  // Functions needed for MeqParmFunklet.
  // By default they throw an exception.
  // <group>
  virtual void update(const ParmData& values);
  virtual void update(const vector<double>& value);
  virtual void update(size_t domain, const vector<double> &unknowns);
  virtual void updateFromTable();
  // </group>

private:
  // A parm cannot be copied, otherwise problems arise with theirParms.
  MeqParm (const MeqParm&);
  MeqParm& operator= (const MeqParm&);

#ifdef EXPR_GRAPH
  virtual std::string getLabel();
#endif

  string        itsName;
  bool          itsIsSolvable;
};



// Envelope class for MeqParm.
// It is used by MeqParmGroup to be able to hold reference counts of a MeqParm.
// It ensures that the underlying MeqExprRep object in MeqExpr is a MeqParm.
class MeqPExpr : public MeqExpr
{
public:
  // Construct from a MeqParm.
  MeqPExpr (MeqParm* ptr)
    : MeqExpr(ptr), itsParmPtr(ptr)
    {}
  // Construct from a MeqExpr which must contain a MeqParm.
  // If not, an exception is thrown.
  MeqPExpr (const MeqExpr&);

  // Functions used on a MeqParm.
  // <group>
  const string& getName() const
    { return itsParmPtr->getName(); }
  int getParmDBSeqNr() const
    { return itsParmPtr->getParmDBSeqNr(); }
  void removeFunklets()
    { itsParmPtr->removeFunklets(); }
  void fillFunklets (const std::map<std::string,LOFAR::ParmDB::ParmValueSet>& parmSet,
             const MeqDomain& domain)
    { itsParmPtr->fillFunklets (parmSet, domain); }
  int initDomain (const vector<MeqDomain>& solveDomains, int& pertIndex,
              vector<int>& scidIndex)
    { return itsParmPtr->initDomain (solveDomains, pertIndex, scidIndex); }
  void setSolvable (bool solvable)
    { itsParmPtr->setSolvable (solvable); }
  bool isSolvable() const
    { return itsParmPtr->isSolvable(); }
  const vector<MeqFunklet*>& getFunklets() const
    { return itsParmPtr->getFunklets(); }
  void save()
    { itsParmPtr->save(); }
  void save(size_t domainIndex)
    { itsParmPtr->save(domainIndex); }
  void update (const ParmData& values)
    { itsParmPtr->update (values); }
  void update (const vector<double>& value)
    { itsParmPtr->update (value); }
  void update(size_t domain, const vector<double> &unknowns)
    { itsParmPtr->update (domain, unknowns); }
  void updateFromTable()
    { itsParmPtr->updateFromTable(); }
  // </group>

private:
  MeqParm* itsParmPtr;
};



class MeqParmGroup
{
public:
  typedef map<string,MeqPExpr>::const_iterator const_iterator;
  typedef map<string,MeqPExpr>::iterator iterator;

  // Default constructor.
  MeqParmGroup();

  // Functions for iteration.
  // <group>
  const_iterator begin() const
    { return itsParms.begin(); }
  const_iterator end() const
    { return itsParms.end(); }
  iterator begin()
    { return itsParms.begin(); }
  iterator end()
    { return itsParms.end(); }
  // </group>

  // Add a Parm object to the map.
  void add (const MeqPExpr&);

  // Find a Parm object.
  // <group>
  const_iterator find (const string& name) const
    { return itsParms.find (name); }
  iterator find (const string& name)
    { return itsParms.find (name); }
  // </group>

  // Clear the group and delete all its parms.
  void clear();

private:
  map<string,MeqPExpr> itsParms;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
