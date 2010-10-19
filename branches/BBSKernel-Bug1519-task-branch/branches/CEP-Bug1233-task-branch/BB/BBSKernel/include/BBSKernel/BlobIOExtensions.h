//# BlobIOExtensions.h: 
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBS_BLOBIOEXTENSIONS_H
#define LOFAR_BBS_BLOBIOEXTENSIONS_H

#include <utility>

namespace casa
{
    //# Forward Declarations
    class Measure;
    class MDirection;
    class MPosition;
    class MeasureHolder;
} // namespace casa
    
namespace LOFAR
{
    //# Forward Declarations
    class BlobOStream;
    class BlobIStream;

    template <typename T, typename U>
    BlobOStream &operator<<(BlobOStream &out, const std::pair<T, U> &obj)
    {
        return out << obj.first << obj.second;
    }

    template <typename T, typename U>
    BlobIStream &operator>>(BlobIStream &in, std::pair<T, U> &obj)
    {
        return in >> obj.first >> obj.second;
    }

    namespace BBS
    {
        BlobOStream &operator<<(BlobOStream &out, const casa::Measure &obj);
        BlobIStream &operator>>(BlobIStream &in, casa::MeasureHolder &obj);
        BlobIStream &operator>>(BlobIStream &in, casa::MDirection &obj);
        BlobIStream &operator>>(BlobIStream &in, casa::MPosition &obj);
    } // namespace BBS
    
} // namespace LOFAR

#endif

