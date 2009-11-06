//# MeasurementAIPS.h: I/O for AIPS++ Measurement Sets.
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBS_BBSKERNEL_MEASUREMENTAIPS_H
#define LOFAR_BBS_BBSKERNEL_MEASUREMENTAIPS_H

#include <BBSKernel/Measurement.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

#include <casa/Arrays/Slicer.h>
#include <ms/MeasurementSets/MeasurementSet.h>

namespace casa
{
    class ROMSObservationColumns;
    class ROMSAntennaColumns;
    class ROMSSpWindowColumns;
    class ROMSPolarizationColumns;
    class ROMSFieldColumns;
};

namespace LOFAR
{
namespace BBS
{
class VisSelection;

class MeasurementAIPS: public Measurement
{
public:
    MeasurementAIPS(const string &filename,
        unsigned int idObservation = 0,
        unsigned int idField = 0,
        unsigned int idDataDescription = 0);

    virtual VisDimensions getDimensions(const VisSelection &selection) const;

    virtual VisData::Pointer read(const VisSelection &selection,
        const string &column = "DATA", bool readUVW = true) const;

    virtual void write(const VisSelection &selection, VisData::Pointer buffer,
        const string &column = "CORRECTED_DATA", bool writeFlags = true);

private:
    void initInstrument();
    void initPhaseCenter();
    void initDimensions();

    casa::Table getTableSelection(const casa::Table &table,
        const VisSelection &selection) const;
    casa::Slicer getCellSlicer(const VisSelection &selection) const;
    VisDimensions getDimensionsImpl(const casa::Table tab_selection,
        const casa::Slicer slicer) const;

    casa::MeasurementSet    itsMS;
    casa::Table             itsMainTableView;

    bool                    itsFreqAxisReversed;

    unsigned int            itsIdObservation;
    unsigned int            itsIdField;
    unsigned int            itsIdDataDescription;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
