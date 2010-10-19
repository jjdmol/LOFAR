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

#include <lofar_config.h>

#include <BBSKernel/BBSKernelStructs.h>
#include <BBSKernel/MNS/MeqDomain.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iostream.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;
    
    ostream& operator<<(ostream& os, const Correlation& obj)
    {
      os << "Correlation:";
      Indent id;
      os << endl << indent << "Selection: " << obj.selection
	 << endl << indent << "Type: "      << obj.type;
      return os;
    }


    ostream& operator<<(ostream& os, const Baselines& obj)
    {
      os << "Baselines:";
      Indent id;
      os << endl << indent << "Station1: " << obj.station1
	 << endl << indent << "Station2: " << obj.station2;
      return os;
    }


    //# -------  BlobOStream operators  ------- #//

    BlobOStream& operator<<(BlobOStream& bos, const Correlation& obj)
    {
      bos << obj.selection
	  << obj.type;
      return bos;
    }


    BlobOStream& operator<<(BlobOStream& bos, const Baselines& obj)
    {
      bos << obj.station1
	  << obj.station2;
      return bos;
    }


    //# -------  BlobIStream operators  ------- #//

    BlobIStream& operator>>(BlobIStream& bis, Correlation& obj)
    {
      int32 selection;
      bis >> obj.selection;
      bis >> obj.type;
      return bis;
    }


    BlobIStream& operator>>(BlobIStream& bis, Baselines& obj)
    {
      bis >> obj.station1
	  >> obj.station2;
      return bis;
    }


  } // namespace BBS

} // namespace LOFAR
