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

#ifndef LOFAR_BBSKERNEL_MEASUREMENTAIPS_H
#define LOFAR_BBSKERNEL_MEASUREMENTAIPS_H

#include <BBSKernel/Measurement.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

#include <casa/Arrays/Slicer.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/ExprNodeSet.h>

#include <Common/ParameterSet.h>

namespace LOFAR
{
namespace BBS
{
class VisSelection;

// \addtogroup BBSKernel
// @{

class MeasurementAIPS: public Measurement
{
public:
    MeasurementAIPS(const string &filename,
        unsigned int idObservation = 0,
        unsigned int idField = 0,
        unsigned int idDataDescription = 0);

    // \name Measurement interface implementation
    // These methods form an implementation of the Measurement interface. See
    // that class for function documentation.
    //
    // @{
    virtual VisDimensions dims(const VisSelection &selection) const;

    virtual VisBuffer::Ptr read(const VisSelection &selection = VisSelection(),
        const string &column = "DATA",
        bool readCovariance = true,
        bool readFlags = true) const;

    virtual void write(VisBuffer::Ptr buffer,
        const VisSelection &selection = VisSelection(),
        const string &column = "CORRECTED_DATA",
        bool writeCovariance = true,
        bool writeFlags = false,
        flag_t flagMask = ~flag_t(0));

    virtual void writeHistory(const ParameterSet &parset) const;

    virtual BaselineMask asMask(const string &filter) const;
    // @}

private:
    void initReferenceDirections();
    void initDimensions();

    void createVisibilityColumn(const string &name);
    void createCovarianceColumn(const string &name);

    casa::MDirection getColumnPhaseReference(const string &column) const;

    casa::Table getVisSelection(casa::Table table,
        const VisSelection &selection) const;
    casa::Table getBaselineSelection(const casa::Table &table,
        const string &pattern) const;
    BaselineMask getBaselineMask(const VisSelection &selection) const;

    Interval<size_t> getChannelRange(const VisSelection &selection) const;
    casa::Slicer getCellSlicer(const VisSelection &selection) const;
    casa::Slicer getCovarianceSlicer(const VisSelection &selection,
        const string &column) const;

    string getLinkedCovarianceColumn(const string &column,
        const string &defaultColumn) const;
    void setLinkedCovarianceColumn(const string &column,
        const string &linkedColumn);

    casa::Array<casa::Float>
    reformatCovarianceArray(const casa::Array<casa::Float> &in,
        unsigned int nCorrelations, unsigned int nFreq, unsigned int nRows)
        const;

    VisDimensions getDimensionsImpl(const casa::Table &tab_selection,
        const casa::Slicer &slicer) const;

    casa::MeasurementSet    itsMS;
    casa::Table             itsMainTableView;

    bool                    itsFreqAxisReversed;

    unsigned int            itsIdObservation;
    unsigned int            itsIdField;
    unsigned int            itsIdDataDescription;
};

Instrument::Ptr readInstrument(const casa::MeasurementSet &ms,
    unsigned int idObservation = 0);
casa::MDirection readPhaseReference(const casa::MeasurementSet &ms,
    unsigned int idField = 0);
casa::MDirection readDelayReference(const casa::MeasurementSet &ms,
    unsigned int idField = 0);
casa::MDirection readTileReference(const casa::MeasurementSet &ms,
    unsigned int idField = 0);
double readFreqReference(const casa::MeasurementSet &ms,
    unsigned int idDataDescription = 0);
// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
