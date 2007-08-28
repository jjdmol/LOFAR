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

#include <BBSKernel/VisSelection.h>
#include <BBSKernel/VisData.h>

#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_set.h>
#include <utility>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

struct Station
{
    string              name;
    casa::MPosition     position;
};

class Instrument
{
public:
    size_t getStationCount() const
    { return stations.size(); }

    string              name;
    casa::MPosition     position;
    vector<Station>     stations;
};

class Measurement
{
public:
    typedef shared_ptr<Measurement> Pointer;

    virtual ~Measurement()
    {}

    virtual VisGrid grid(const VisSelection &selection) const = 0;

    virtual VisData::Pointer read(const VisSelection &selection,
        const string &column = "DATA", bool readUVW = true) const = 0;

    virtual void write(const VisSelection &selection, VisData::Pointer buffer,
        const string &column = "CORRECTED_DATA",
        bool writeFlags = true) = 0;

    const Instrument &getInstrument() const
    { return itsInstrument; }

    const casa::MDirection &getPhaseCenter() const
    { return itsPhaseCenter; }

    pair<casa::MEpoch, casa::MEpoch> getTimeRange() const
    { return itsTimeRange; }

    const cell_centered_axis<regular_series> &getSpectrum() const
    { return itsSpectrum; }

    pair<double, double> getFreqRange() const
    { return itsSpectrum.range(); }

    size_t getChannelCount() const
    { return itsSpectrum.size(); }

    const vector<string> &getPolarizations() const
    { return itsPolarizations; }

    size_t getPolarizationCount() const
    { return itsPolarizations.size(); }

protected:
    Instrument                          itsInstrument;
    casa::MDirection                    itsPhaseCenter;
    pair<casa::MEpoch, casa::MEpoch>    itsTimeRange;
    cell_centered_axis<regular_series>  itsSpectrum;
    vector<string>                      itsPolarizations;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
