//# BlobIOExtensions.cc: 
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

#include <lofar_config.h>
#include <BBSKernel/BlobIOExtensions.h>
#include <BBSKernel/Exceptions.h>

#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobAipsIO.h>
#include <Blob/BlobSTL.h>

#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/Measure.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>

#include <casa/Containers/Record.h>
#include <casa/IO/AipsIO.h>

namespace LOFAR
{
namespace BBS
{

    BlobOStream &operator<<(BlobOStream &out, const casa::Measure &obj)
    {
        casa::MeasureHolder measure(obj);

        BlobAipsIO aipsBuffer(out);
        casa::AipsIO aipsStream(&aipsBuffer);
        casa::String aipsErrorMessage;
        casa::Record aipsRecord;

        if(!measure.toRecord(aipsErrorMessage, aipsRecord))
        {
            THROW(BBSKernelException, "Unable to serialise Measure ("
                << aipsErrorMessage << ")");
        }        
        aipsStream << aipsRecord;

        return out;
    }

    BlobIStream &operator>>(BlobIStream &in, casa::MeasureHolder &obj)
    {
        BlobAipsIO aipsBuffer(in);
        casa::AipsIO aipsStream(&aipsBuffer);
        casa::String aipsErrorMessage;
        casa::Record aipsRecord;

        aipsStream >> aipsRecord;        
        if(!obj.fromRecord(aipsErrorMessage, aipsRecord))
        {
            THROW(BBSKernelException, "Unable to deserialise MeasureHolder ("
                << aipsErrorMessage << ")");
        }
        
        return in;
    }

    BlobIStream &operator>>(BlobIStream &in, casa::MDirection &obj)
    {
        casa::MeasureHolder measure;
        
        in >> measure;
        obj = measure.asMDirection();
        return in;
    }

    BlobIStream &operator>>(BlobIStream &in, casa::MPosition &obj)
    {
        casa::MeasureHolder measure;
        
        in >> measure;
        obj = measure.asMPosition();
        return in;
    }

} // namespace BBS
} // namespace LOFAR

