//# ParmData.h: The properties for solvable parameters
//#
//# Copyright (C) 2004
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

#ifndef LOFAR_BB_BBS_PARMDATA_H
#define LOFAR_BB_BBS_PARMDATA_H

// \file
// The properties for solvable parameters

//# Includes
#include <BBSKernel/MNS/MeqMatrix.h>
#include <BBSKernel/MNS/MeqFunklet.h>
#include <Common/LofarTypes.h>
#include <string>
#include <vector>

namespace LOFAR {

//# Forward Declarations.
class BlobOStream;
class BlobIStream;

namespace BBS
{

// \addtogroup BBSKernel
// @{

class ParmData
{
public:
  ParmData();
  // Constructor.
  ParmData (const std::string& name,
	    int parmDBSeqNr);

  ParmData (const std::string& name,
	    int parmDBSeqNr,
	    const std::vector<MeqFunklet*>& values);

  // Get the properties.
  // <group>
  const std::string& getName() const
    { return itsName; }
  int getParmDBSeqNr() const
    { return itsDBSeqNr; }
  int getNrPert() const
    { return itsNrPert; }

  // Get the number of values.
  int size() const
    { return itsInfo.size(); }
  
  // Get the coefficients for the i-th solve domain.
  const MeqMatrix& getCoeff (int index) const
    { return itsInfo[index].coeff; }
  
  // Get the solvable mask for the i-th solve domain.
  const std::vector<bool> &getSolvMask(int index) const
    { return itsInfo[index].solvMask; }
    
  // Append the solvable coefficient values to the vector.
  // Set the nr of scids and the first scid index.
  int setValues (uint solveDomainInx, vector<double>& values);

  // Update the values that are solvable.
  // Return the numbers of values updated (i.e. solvable values).
  int update (uint solveDomainInx, const vector<double>& values);

  // Check if two objects are equal.
  // They are if the name is equal.
  bool operator== (const ParmData& other);

  // Write the object into a blob.
  friend BlobOStream& operator<< (BlobOStream&, const ParmData&);

  // Read the object from a blob.
  friend BlobIStream& operator>> (BlobIStream&, ParmData&);

  // Write the object.
  friend ostream& operator<< (ostream&, const ParmData&);
  // Write the object in more detail.
  void show (ostream& os) const;

  // This struct describes the solvable parm data for one solve domain.
  struct PDInfo
  {
    MeqMatrix         coeff;
    std::vector<bool> solvMask;
    int32             nscid;
    int32             firstScid;

    PDInfo() : nscid(0), firstScid(-1) {}

    // Write the object into a blob.
    friend BlobOStream& operator<< (BlobOStream&, const PDInfo&);
    // Read the object from a blob.
    friend BlobIStream& operator>> (BlobIStream&, PDInfo&);
  };

  // Get access to the info objects.
  // <group>
  vector<PDInfo>& info()
    { return itsInfo; }
  const vector<PDInfo>& info() const
    { return itsInfo; }
  // </group>

private:
  std::string         itsName;       //# parm name
  int32               itsDBSeqNr;    //# 
  int32               itsNrPert;     //# nr of perturbed values
  std::vector<PDInfo> itsInfo;       //# info per solve domain
};


// Describe the solvable parm data for all parameters and solve domain.
class ParmDataInfo
{
public:
  // Default constructor.
  ParmDataInfo()
    {}

  // Is there any parm or domain defined?
  bool empty() const
    { return itsParmData.size() == 0  ||  itsDomainIndices.size() == 0; }

  // Get access to the ParmData vector.
  // <group>
  std::vector<ParmData>& parms()
    { return itsParmData; }
  const std::vector<ParmData>& parms() const
    { return itsParmData; }
  // </group>

  // Set the solve domains used by a Prediffer.
  void set (const std::vector<MeqDomain>& solveDomains,
	    const MeqDomain& workDomain);

  // Get the solve domains.
  // <group>
  const std::vector<MeqDomain>& getDomains() const
    { return itsDomains; }
  const std::vector<int32>& getDomainIndices() const
    { return itsDomainIndices; }
  // </group>

  // Write the object into a blob (itsDomains is not written).
  friend BlobOStream& operator<< (BlobOStream&, const ParmDataInfo&);
  // Read the object from a blob.
  friend BlobIStream& operator>> (BlobIStream&, ParmDataInfo&);

  // Write the object.
  friend ostream& operator<< (ostream&, const ParmDataInfo&);
  // Write the object in more detail.
  void show (ostream& os) const;

private:
  // Forbid copying (not needed and expensive).
  // <group>
  ParmDataInfo (const ParmDataInfo&);
  ParmDataInfo& operator= (const ParmDataInfo&);
  // </group>

  std::vector<ParmData>  itsParmData;
  std::vector<MeqDomain> itsDomains;
  std::vector<int32>     itsDomainIndices;
};


// @}

} // namespace BBS
} // namespace LOFAR

#endif
