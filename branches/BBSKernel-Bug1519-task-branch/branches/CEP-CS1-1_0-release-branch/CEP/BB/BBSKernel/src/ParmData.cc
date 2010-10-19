//# ParmData.cc: The properties for solvable parameters
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

#include <lofar_config.h>
#include <BBSKernel/ParmData.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{
  using LOFAR::operator<<;

  ParmData::ParmData()
    : itsDBSeqNr   (-1),
      itsNrPert    (0)
  {}

  ParmData::ParmData (const std::string& name,
		      int parmDBSeqNr)
    : itsName    (name),
      itsDBSeqNr (parmDBSeqNr),
      itsNrPert  (0)
  {}

  ParmData::ParmData (const std::string& name,
		      int parmDBSeqNr,
		      const std::vector<MeqFunklet*>& funks)
    : itsName    (name),
      itsDBSeqNr (parmDBSeqNr),
      itsNrPert  (0)
  {
    int nrv = funks.size();
    itsInfo.resize (nrv);
    for (int i=0; i<nrv; ++i) {
      // Copy the data of each funklet.
      PDInfo& info = itsInfo[i];
      info.coeff     = funks[i]->getCoeff();
      info.solvMask  = funks[i]->getSolvMask();
      info.nscid     = funks[i]->nscid();
      info.firstScid = -1;
    }
  }

  int ParmData::setValues (uint solveDomainInx, vector<double>& values)
  {
    DBGASSERT (solveDomainInx < itsInfo.size());
    PDInfo& info = itsInfo[solveDomainInx];
    info.firstScid = values.size();
    double* val = info.coeff.doubleStorage();
    const vector<bool>& mask = info.solvMask;
    // Add the solvable values to the vector.
    // So ignore all unsolvable coefficients.
    int nr = 0;
    for (int j=0; j<info.coeff.nelements(); ++j) {
      if (mask[j]) {
	values.push_back (val[j]);
	nr++;
      }
    }
    info.nscid = nr;
    return nr;
  }

  int ParmData::update (uint solveDomainInx, const vector<double>& values)
  {
    DBGASSERT (solveDomainInx < itsInfo.size());
    PDInfo& info = itsInfo[solveDomainInx];
    double* val = info.coeff.doubleStorage();
    const double* svalues = &values[info.firstScid];
    const vector<bool>& mask = info.solvMask;
    // The values vector only contains the solvable values.
    // So ignore all unsolvable coefficients.
    int nr = 0;
    for (int j=0; j<info.coeff.nelements(); ++j) {
      if (mask[j]) {
	val[j] = svalues[nr++];
      }
    }
    DBGASSERT (nr == info.nscid);
    return nr;
  }

  bool ParmData::operator== (const ParmData& other)
  {
    if (itsName   != other.itsName)   return false;
    return true;
  }


  void ParmDataInfo::set (const vector<MeqDomain>& solveDomains,
			  const MeqDomain& workDomain)
  {
    // solveDomains contains all solve domains for the entire solve.
    // Only select those that overlap the given work domain.
    itsDomains.clear();
    itsDomains.reserve (solveDomains.size());
    itsDomainIndices.clear();
    itsDomainIndices.reserve (solveDomains.size());
    for (uint i=0; i<solveDomains.size(); ++i) {
      if (solveDomains[i].startX() < workDomain.endX()  &&
	  solveDomains[i].endX() > workDomain.startX()  &&
	  solveDomains[i].startY() < workDomain.endY()  &&
	  solveDomains[i].endY() > workDomain.startY()) {
	itsDomains.push_back (solveDomains[i]);
	itsDomainIndices.push_back (i);
      }
    }
    itsParmData.clear();
  }


  BlobOStream& operator<< (BlobOStream& bos, const ParmData& parm)
  {
    bos << parm.itsName
	<< parm.itsDBSeqNr
	<< parm.itsNrPert;
    bos << int32(parm.itsInfo.size());
    for (uint i=0; i<parm.itsInfo.size(); ++i) {
      bos << parm.itsInfo[i];
    }
    return bos;
  }

  // Read the object from a blob.
  BlobIStream& operator>> (BlobIStream& bis, ParmData& parm)
  {
    int32 sz;
    bis >> parm.itsName
	>> parm.itsDBSeqNr
	>> parm.itsNrPert;
    bis >> sz;
    parm.itsInfo.resize (sz);
    for (int i=0; i<sz; ++i) {
      bis >> parm.itsInfo[i];
    }
    return bis;
  }

  ostream& operator<< (ostream& os, const ParmData& parm)
  {
    os << parm.itsName << '(' << parm.itsNrPert << ')';
    return os;
  }

  void ParmData::show (ostream& os) const
  {
    os << ' ' << itsName << '(' << itsNrPert << ')';
    for (uint i=0; i<itsInfo.size(); ++i) {
      if (itsInfo[i].coeff.isNull()) {
	os << " no-coeffs";
      } else {
	os << ' ' << itsInfo[i].coeff << 's';
      }
      os << itsInfo[i].nscid << ',' << itsInfo[i].firstScid;
    }
    os << endl;
  }


  BlobOStream& operator<< (BlobOStream& bos, const ParmData::PDInfo& parm)
  {
    bos << parm.coeff;
    bos.put (parm.solvMask);
    bos << parm.nscid << parm.firstScid;
    return bos;
  }

  BlobIStream& operator>> (BlobIStream& bis, ParmData::PDInfo& parm)
  {
    bis >> parm.coeff;
    bis.get (parm.solvMask);
    bis >> parm.nscid >> parm.firstScid;
    return bis;
  }


  BlobOStream& operator<< (BlobOStream& bos, const ParmDataInfo& parm)
  {
    bos.putStart ("ParmDataInfo", 1);
    bos << int32(parm.itsParmData.size());
    for (uint i=0; i<parm.itsParmData.size(); ++i) {
      bos << parm.itsParmData[i];
    }
    bos.put (parm.itsDomainIndices);
    bos.putEnd();
    return bos;
  }

  // Read the object from a blob.
  BlobIStream& operator>> (BlobIStream& bis, ParmDataInfo& parm)
  {
    bis.getStart ("ParmDataInfo");
    int32 sz;
    bis >> sz;
    parm.itsParmData.resize (sz);
    for (int i=0; i<sz; ++i) {
      bis >> parm.itsParmData[i];
    }
    bis.get (parm.itsDomainIndices);
    bis.getEnd();
    return bis;
  }

  ostream& operator<< (ostream& os, const ParmDataInfo& parm)
  {
    os << parm.parms();
    return os;
  }

  void ParmDataInfo::show (ostream& os) const
  {
    os << "ParmData:" << endl;
    for (uint i=0; i<itsParmData.size(); ++i) {
      itsParmData[i].show (os);
    }
    os << "Domains:" << endl;
    for (uint i=0; i<itsDomains.size(); ++i) {
      os << ' ' << itsDomains[i] << " [" << itsDomainIndices[i] << ']' << endl;
    }
  }

} // namespace BBS
} // namespace LOFAR
