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
    typedef shared_ptr<MeasurementAIPS>         Ptr;
    typedef shared_ptr<const MeasurementAIPS>   ConstPtr;   
   
    MeasurementAIPS(const string &filename,
        unsigned int idObservation = 0,
        unsigned int idField = 0,
        unsigned int idDataDescription = 0);           // create clearCal columns "MODEL_DATA" and "CORRECTED_DATA"

    // \name Measurement interface implementation
    // These methods form an implementation of the Measurement interface. See
    // that class for function documentation.
    //
    // @{
    virtual VisDimensions dimensions(const VisSelection &selection) const;

    virtual VisBuffer::Ptr read(const VisSelection &selection = VisSelection(),
        const string &column = "DATA") const;

    virtual void write(VisBuffer::Ptr buffer,
        const VisSelection &selection = VisSelection(),
        const string &column = "CORRECTED_DATA", bool writeFlags = true,
        flag_t flagMask = ~flag_t(0));
    
    virtual BaselineMask asMask(const string &filter) const;
    // @}

    // add MODEL_DATA and/or CORRECTED_DATA according to itsClearcalColFlag
    void addClearcalColumns();   
    

private:
    void initInstrument();
    void initPhaseReference();
    void initDimensions();

    bool hasColumn(const string &column) const;
    void addDataColumn(const string &column);
    
    casa::Table getVisSelection(casa::Table table,
        const VisSelection &selection) const;
    casa::Table getBaselineSelection(const casa::Table &table,
        const string &pattern) const;
    BaselineMask getBaselineMask(const VisSelection &selection) const;
    casa::Slicer getCellSlicer(const VisSelection &selection) const;
    VisDimensions getDimensionsImpl(const casa::Table &tab_selection,
        const casa::Slicer &slicer) const;

    casa::MeasurementSet    itsMS;
    casa::Table             itsMainTableView;

    bool                    itsFreqAxisReversed;

    unsigned int            itsIdObservation;
    unsigned int            itsIdField;
    unsigned int            itsIdDataDescription;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
