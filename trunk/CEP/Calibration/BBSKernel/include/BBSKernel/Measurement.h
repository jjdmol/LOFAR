//# Measurement.h: Interface class for measurement I/O.
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBS_BBSKERNEL_MEASUREMENT_H
#define LOFAR_BBS_BBSKERNEL_MEASUREMENT_H

#include <BBSKernel/Instrument.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/VisDimensions.h>
#include <BBSKernel/VisSelection.h>

#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{
    class Measurement
    {
    public:
        typedef shared_ptr<Measurement> Pointer;

        virtual ~Measurement()
        {
        }

        virtual VisDimensions
            getDimensions(const VisSelection &selection) const = 0;

        virtual VisData::Pointer read(const VisSelection &selection,
            const string &column = "DATA", bool readUVW = true) const = 0;

        virtual void write(const VisSelection &selection,
            VisData::Pointer buffer, const string &column = "CORRECTED_DATA",
            bool writeFlags = true) = 0;

        const Instrument &getInstrument() const
        { return itsInstrument; }

        const casa::MDirection &getPhaseCenter() const
        { return itsPhaseCenter; }

        const VisDimensions &getDimensions() const
        { return itsDimensions; }

    protected:
        Instrument              itsInstrument;
        casa::MDirection        itsPhaseCenter;
        VisDimensions           itsDimensions;
    };

} //# namespace BBS
} //# namespace LOFAR

#endif
