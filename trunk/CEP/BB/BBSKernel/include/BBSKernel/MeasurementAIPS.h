//# MeasurementAIPS.h: 
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

#ifndef LOFAR_BBS_BBSKERNEL_MEASUREMENTAIPS_H
#define LOFAR_BBS_BBSKERNEL_MEASUREMENTAIPS_H

#include <BBSKernel/Measurement.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

#include <casa/Arrays/Slicer.h>
#include <ms/MeasurementSets/MeasurementSet.h>

namespace LOFAR
{
namespace BBS
{
class VisSelection;

class MeasurementAIPS: public Measurement
{
public:
    MeasurementAIPS(string filename,
        uint observationId = 0,
        uint dataDescriptionId = 0,
        uint fieldId = 0);

    ~MeasurementAIPS();

    virtual VisGrid grid(const VisSelection &selection) const;

    virtual VisData::Pointer read(const VisSelection &selection,
        const string &column = "DATA",
        bool readUVW = true) const;

    virtual void write(const VisSelection &selection,
        VisData::Pointer buffer,
        const string &column = "CORRECTED_DATA",
        bool writeFlags = true) const;

private:
    Instrument readInstrumentInfo
        (const casa::MSAntenna &tab_antenna,
        const casa::MSObservation &tab_observation,
        uint id);

    vector<string> readPolarizationInfo
        (const casa::MSPolarization &tab_polarization, uint id);

    void readSpectralInfo
        (const casa::MSSpectralWindow &tab_spectralWindow, uint id);

    casa::MDirection readFieldInfo(const casa::MSField &tab_field, uint id);

    casa::TableExprNode getTAQLExpression(const VisSelection &selection) const;

    casa::Slicer getCellSlicer(const VisSelection &selection) const;

    VisGrid getGrid(const casa::Table tab_selection, const casa::Slicer slicer)
        const;

    VisData::Pointer allocate(const VisGrid &grid) const;

    string                  itsFilename;
    casa::MeasurementSet    itsMS;

    casa::Int               itsObservationId, itsDataDescriptionId,
                            itsFieldId;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
