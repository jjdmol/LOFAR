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

#include <BBS3/ParmData.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>

namespace LOFAR {

  ParmData::ParmData (const std::string& name, int nrSpid, int firstSpid,
		      const MeqMatrix& values)
    : itsNrSpid    (nrSpid),
      itsFirstSpid (firstSpid),
      itsName      (name),
      itsValues    (values)
  {}

  BlobOStream& operator<< (BlobOStream& bos, const ParmData& parm)
  {
    bos << parm.itsName << parm.itsNrSpid << parm.itsFirstSpid
	<< parm.itsValues;
    return bos;
  }

  // Read the object from a blob.
  BlobIStream& operator>> (BlobIStream& bis, ParmData& parm)
  {
    bis >> parm.itsName >> parm.itsNrSpid >> parm.itsFirstSpid
	>> parm.itsValues;
    return bis;
  }

}
