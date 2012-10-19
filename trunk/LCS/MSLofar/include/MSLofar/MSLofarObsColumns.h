//# MSLofarObsColumns.h: provides easy access to LOFAR's MSObservation columns
//# Copyright (C) 2011
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
//#
//# @author Ger van Diepen

#ifndef MSLOFAR_MSLOFAROBSCOLUMNS_H
#define MSLOFAR_MSLOFAROBSCOLUMNS_H

#include <ms/MeasurementSets/MSObsColumns.h>

namespace LOFAR {

  //# Forward Declaration
  class MSLofarObservation;

  // This class provides read-only access to the columns in the MSLofarObservation
  // Table. It does the declaration of all the Scalar and ArrayColumns with the
  // correct types, so the application programmer doesn't have to worry about
  // getting those right. There is an access function for every predefined
  // column. Access to non-predefined columns will still have to be done with
  // explicit declarations.

  class ROMSLofarObservationColumns: public casa::ROMSObservationColumns
  {
  public:

    // Create a columns object that accesses the data in the specified Table.
    ROMSLofarObservationColumns(const MSLofarObservation& msLofarObservation);

    // The destructor does nothing special.
    ~ROMSLofarObservationColumns();

    // Access to columns.
    // <group>
    const casa::ROScalarColumn<casa::String>& projectTitle() const
      { return projectTitle_p; }
    const casa::ROScalarColumn<casa::String>& projectPI() const
      { return projectPI_p; }
    const casa::ROArrayColumn<casa::String>& projectCoI() const
      { return projectCoI_p; }
    const casa::ROScalarColumn<casa::String>& projectContact() const
      { return projectContact_p; }
    const casa::ROScalarColumn<casa::String>& observationId() const
      { return observationId_p; }
    const casa::ROScalarColumn<casa::Double>& observationStart() const
      { return observationStart_p; }
    const casa::ROScalarColumn<casa::Double>& observationEnd() const
      { return observationEnd_p; }
    const casa::ROScalarColumn<casa::Double>& observationFrequencyMax() const
      { return observationFrequencyMax_p; }
    const casa::ROScalarColumn<casa::Double>& observationFrequencyMin() const
      { return observationFrequencyMin_p; }
    const casa::ROScalarColumn<casa::Double>& observationFrequencyCenter() const
      { return observationFrequencyCenter_p; }
    const casa::ROScalarColumn<casa::Int>& subArrayPointing() const
      { return subArrayPointing_p; }
    const casa::ROScalarColumn<casa::Int>& nofBitsPerSample() const
      { return nofBitsPerSample_p; }
    const casa::ROScalarColumn<casa::String>& antennaSet() const
      { return antennaSet_p; }
    const casa::ROScalarColumn<casa::String>& filterSelection() const
      { return filterSelection_p; }
    const casa::ROScalarColumn<casa::Double>& clockFrequency() const
      { return clockFrequency_p; }
    const casa::ROArrayColumn<casa::String>& target() const
      { return target_p; }
    const casa::ROScalarColumn<casa::String>& systemVersion() const
      { return systemVersion_p; }
    const casa::ROScalarColumn<casa::String>& pipelineName() const
      { return pipelineName_p; }
    const casa::ROScalarColumn<casa::String>& pipelineVersion() const
      { return pipelineVersion_p; }
    const casa::ROScalarColumn<casa::String>& filename() const
      { return filename_p; }
    const casa::ROScalarColumn<casa::String>& filetype() const
      { return filetype_p; }
    const casa::ROScalarColumn<casa::Double>& filedate() const
      { return filedate_p; }
    // </group>

    // Access to Quantity columns
    // <group>
    const casa::ROScalarQuantColumn<casa::Double>& observationStartQuant() const 
      { return observationStartQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationEndQuant() const 
      { return observationEndQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationFrequencyMaxQuant() const 
      { return observationFrequencyMaxQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationFrequencyMinQuant() const 
      { return observationFrequencyMinQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& clockFrequencyQuant() const 
      { return clockFrequencyQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationFrequencyCenterQuant() const 
      { return observationFrequencyCenterQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& filedateQuant() const 
      { return filedateQuant_p; }
    // </group>

    // Access to Measure columns
    // <group>
    const casa::ROScalarMeasColumn<casa::MEpoch>& observationStartMeas() const 
      { return observationStartMeas_p; }
    const casa::ROScalarMeasColumn<casa::MEpoch>& observationEndMeas() const 
      { return observationEndMeas_p; }
    const casa::ROScalarMeasColumn<casa::MEpoch>& filedateMeas() const 
      { return filedateMeas_p; }
    // </group>

  protected:
    //# Default constructor creates a object that is not usable. Use the attach
    //# function correct this.
    ROMSLofarObservationColumns();

    //# Attach this object to the supplied table.
    void attach (const MSLofarObservation& msLofarObservation);

  private:
    //# Make the assignment operator and the copy constructor private to prevent
    //# any compiler generated one from being used.
    ROMSLofarObservationColumns(const ROMSLofarObservationColumns&);
    ROMSLofarObservationColumns& operator=(const ROMSLofarObservationColumns&);

    //# required columns
    casa::ROScalarColumn<casa::String> projectTitle_p;
    casa::ROScalarColumn<casa::String> projectPI_p;
    casa::ROArrayColumn<casa::String>  projectCoI_p;
    casa::ROScalarColumn<casa::String> projectContact_p;
    casa::ROScalarColumn<casa::String> observationId_p;
    casa::ROScalarColumn<casa::Double> observationStart_p;
    casa::ROScalarColumn<casa::Double> observationEnd_p;
    casa::ROScalarColumn<casa::Double> observationFrequencyMax_p;
    casa::ROScalarColumn<casa::Double> observationFrequencyMin_p;
    casa::ROScalarColumn<casa::Double> observationFrequencyCenter_p;
    casa::ROScalarColumn<casa::Int>    subArrayPointing_p;
    casa::ROScalarColumn<casa::Int>    nofBitsPerSample_p;
    casa::ROScalarColumn<casa::String> antennaSet_p;
    casa::ROScalarColumn<casa::String> filterSelection_p;
    casa::ROScalarColumn<casa::Double> clockFrequency_p;
    casa::ROArrayColumn<casa::String>  target_p;
    casa::ROScalarColumn<casa::String> systemVersion_p;
    casa::ROScalarColumn<casa::String> pipelineName_p;
    casa::ROScalarColumn<casa::String> pipelineVersion_p;
    casa::ROScalarColumn<casa::String> filename_p;
    casa::ROScalarColumn<casa::String> filetype_p;
    casa::ROScalarColumn<casa::Double> filedate_p;
    //# Access to Quantum columns
    casa::ROScalarQuantColumn<casa::Double> observationStartQuant_p;
    casa::ROScalarQuantColumn<casa::Double> observationEndQuant_p;
    casa::ROScalarQuantColumn<casa::Double> observationFrequencyMaxQuant_p;
    casa::ROScalarQuantColumn<casa::Double> observationFrequencyMinQuant_p;
    casa::ROScalarQuantColumn<casa::Double> observationFrequencyCenterQuant_p;
    casa::ROScalarQuantColumn<casa::Double> clockFrequencyQuant_p;
    casa::ROScalarQuantColumn<casa::Double> filedateQuant_p;
    //# Access to Measure columns
    casa::ROScalarMeasColumn<casa::MEpoch> observationStartMeas_p;
    casa::ROScalarMeasColumn<casa::MEpoch> observationEndMeas_p;
    casa::ROScalarMeasColumn<casa::MEpoch> filedateMeas_p;
  };


  // This class provides read/write access to the columns in the MSLofarObservation
  // Table. It does the declaration of all the Scalar and ArrayColumns with the
  // correct types, so the application programmer doesn't have to
  // worry about getting those right. There is an access function
  // for every predefined column. Access to non-predefined columns will still
  // have to be done with explicit declarations.

  class MSLofarObservationColumns: public casa::MSObservationColumns
  {
  public:

    // Create a columns object that accesses the data in the specified Table.
    MSLofarObservationColumns(MSLofarObservation& msLofarObservation);

    // The destructor does nothing special.
    ~MSLofarObservationColumns();

    // Readonly access to columns.
    // <group>
    const casa::ROScalarColumn<casa::String>& projectTitle() const
      { return roProjectTitle_p; }
    const casa::ROScalarColumn<casa::String>& projectPI() const
      { return roProjectPI_p; }
    const casa::ROArrayColumn<casa::String>& projectCoI() const
      { return roProjectCoI_p; }
    const casa::ROScalarColumn<casa::String>& projectContact() const
      { return roProjectContact_p; }
    const casa::ROScalarColumn<casa::String>& observationId() const
      { return roObservationId_p; }
    const casa::ROScalarColumn<casa::Double>& observationStart() const
      { return roObservationStart_p; }
    const casa::ROScalarColumn<casa::Double>& observationEnd() const
      { return roObservationEnd_p; }
    const casa::ROScalarColumn<casa::Double>& observationFrequencyMax() const
      { return roObservationFrequencyMax_p; }
    const casa::ROScalarColumn<casa::Double>& observationFrequencyMin() const
      { return roObservationFrequencyMin_p; }
    const casa::ROScalarColumn<casa::Double>& observationFrequencyCenter() const
      { return roObservationFrequencyCenter_p; }
    const casa::ROScalarColumn<casa::Int>& subArrayPointing() const
      { return roSubArrayPointing_p; }
    const casa::ROScalarColumn<casa::Int>& nofBitsPerSample() const
      { return roNofBitsPerSample_p; }
    const casa::ROScalarColumn<casa::String>& antennaSet() const
      { return roAntennaSet_p; }
    const casa::ROScalarColumn<casa::String>& filterSelection() const
      { return roFilterSelection_p; }
    const casa::ROScalarColumn<casa::Double>& clockFrequency() const
      { return roClockFrequency_p; }
    const casa::ROArrayColumn<casa::String>& target() const
      { return roTarget_p; }
    const casa::ROScalarColumn<casa::String>& systemVersion() const
      { return roSystemVersion_p; }
    const casa::ROScalarColumn<casa::String>& pipelineName() const
      { return roPipelineName_p; }
    const casa::ROScalarColumn<casa::String>& pipelineVersion() const
      { return roPipelineVersion_p; }
    const casa::ROScalarColumn<casa::String>& filename() const
      { return roFilename_p; }
    const casa::ROScalarColumn<casa::String>& filetype() const
      { return roFiletype_p; }
    const casa::ROScalarColumn<casa::Double>& filedate() const
      { return roFiledate_p; }
    // </group>

    // Readonly access to Quantity columns
    // <group>
    const casa::ROScalarQuantColumn<casa::Double>& observationStartQuant() const 
      { return roObservationStartQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationEndQuant() const 
      { return roObservationEndQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationFrequencyMaxQuant() const 
      { return roObservationFrequencyMaxQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationFrequencyMinQuant() const 
      { return roObservationFrequencyMinQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& observationFrequencyCenterQuant() const 
      { return roObservationFrequencyCenterQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& clockFrequencyQuant() const 
      { return roClockFrequencyQuant_p; }
    const casa::ROScalarQuantColumn<casa::Double>& filedateQuant() const 
      { return roFiledateQuant_p; }
    // </group>

    // Readonly access to Measure columns
    // <group>
    const casa::ROScalarMeasColumn<casa::MEpoch>& observationStartMeas() const 
      { return roObservationStartMeas_p; }
    const casa::ROScalarMeasColumn<casa::MEpoch>& observationEndMeas() const 
      { return roObservationEndMeas_p; }
    const casa::ROScalarMeasColumn<casa::MEpoch>& filedateMeas() const 
      { return roFiledateMeas_p; }
    // </group>

    // Read/write access to columns.
    // <group>
    casa::ScalarColumn<casa::String>& projectTitle()
      { return rwProjectTitle_p; }
    casa::ScalarColumn<casa::String>& projectPI()
      { return rwProjectPI_p; }
    casa::ArrayColumn<casa::String>& projectCoI()
      { return rwProjectCoI_p; }
    casa::ScalarColumn<casa::String>& projectContact()
      { return rwProjectContact_p; }
    casa::ScalarColumn<casa::String>& observationId()
      { return rwObservationId_p; }
    casa::ScalarColumn<casa::Double>& observationStart()
      { return rwObservationStart_p; }
    casa::ScalarColumn<casa::Double>& observationEnd()
      { return rwObservationEnd_p; }
    casa::ScalarColumn<casa::Double>& observationFrequencyMax()
      { return rwObservationFrequencyMax_p; }
    casa::ScalarColumn<casa::Double>& observationFrequencyMin()
      { return rwObservationFrequencyMin_p; }
    casa::ScalarColumn<casa::Double>& observationFrequencyCenter()
      { return rwObservationFrequencyCenter_p; }
    casa::ScalarColumn<casa::Int>& subArrayPointing()
      { return rwSubArrayPointing_p; }
    casa::ScalarColumn<casa::Int>& nofBitsPerSample()
      { return rwNofBitsPerSample_p; }
    casa::ScalarColumn<casa::String>& antennaSet()
      { return rwAntennaSet_p; }
    casa::ScalarColumn<casa::String>& filterSelection()
      { return rwFilterSelection_p; }
    casa::ScalarColumn<casa::Double>& clockFrequency()
      { return rwClockFrequency_p; }
    casa::ArrayColumn<casa::String>& target()
      { return rwTarget_p; }
    casa::ScalarColumn<casa::String>& systemVersion()
      { return rwSystemVersion_p; }
    casa::ScalarColumn<casa::String>& pipelineName()
      { return rwPipelineName_p; }
    casa::ScalarColumn<casa::String>& pipelineVersion()
      { return rwPipelineVersion_p; }
    casa::ScalarColumn<casa::String>& filename()
      { return rwFilename_p; }
    casa::ScalarColumn<casa::String>& filetype()
      { return rwFiletype_p; }
    casa::ScalarColumn<casa::Double>& filedate()
      { return rwFiledate_p; }
    // </group>

    // Read/write access to Quantity columns
    // <group>
    casa::ScalarQuantColumn<casa::Double>& observationStartQuant() 
      { return rwObservationStartQuant_p; }
    casa::ScalarQuantColumn<casa::Double>& observationEndQuant() 
      { return rwObservationEndQuant_p; }
    casa::ScalarQuantColumn<casa::Double>& observationFrequencyMaxQuant() 
      { return rwObservationFrequencyMaxQuant_p; }
    casa::ScalarQuantColumn<casa::Double>& observationFrequencyMinQuant() 
      { return rwObservationFrequencyMinQuant_p; }
    casa::ScalarQuantColumn<casa::Double>& observationFrequencyCenterQuant() 
      { return rwObservationFrequencyCenterQuant_p; }
    casa::ScalarQuantColumn<casa::Double>& clockFrequencyQuant() 
      { return rwClockFrequencyQuant_p; }
    casa::ScalarQuantColumn<casa::Double>& filedateQuant() 
      { return rwFiledateQuant_p; }
    // </group>

    // Read/write access to Measure columns
    // <group>
    casa::ScalarMeasColumn<casa::MEpoch>& observationStartMeas() 
      { return rwObservationStartMeas_p; }
    casa::ScalarMeasColumn<casa::MEpoch>& observationEndMeas() 
      { return rwObservationEndMeas_p; }
    casa::ScalarMeasColumn<casa::MEpoch>& filedateMeas() 
      { return rwFiledateMeas_p; }
    // </group>

  protected:
    //# Default constructor creates a object that is not usable. Use the attach
    //# function correct this.
    MSLofarObservationColumns();

    //# Attach this object to the supplied table.
    void attach(MSLofarObservation& msLofarObservation);

  private:
    //# Make the assignment operator and the copy constructor private to prevent
    //# any compiler generated one from being used.
    MSLofarObservationColumns(const MSLofarObservationColumns&);
    MSLofarObservationColumns& operator=(const MSLofarObservationColumns&);

    //# required columns
    casa::ROScalarColumn<casa::String> roProjectTitle_p;
    casa::ROScalarColumn<casa::String> roProjectPI_p;
    casa::ROArrayColumn<casa::String>  roProjectCoI_p;
    casa::ROScalarColumn<casa::String> roProjectContact_p;
    casa::ROScalarColumn<casa::String> roObservationId_p;
    casa::ROScalarColumn<casa::Double> roObservationStart_p;
    casa::ROScalarColumn<casa::Double> roObservationEnd_p;
    casa::ROScalarColumn<casa::Double> roObservationFrequencyMax_p;
    casa::ROScalarColumn<casa::Double> roObservationFrequencyMin_p;
    casa::ROScalarColumn<casa::Double> roObservationFrequencyCenter_p;
    casa::ROScalarColumn<casa::Int>    roSubArrayPointing_p;
    casa::ROScalarColumn<casa::Int>    roNofBitsPerSample_p;
    casa::ROScalarColumn<casa::String> roAntennaSet_p;
    casa::ROScalarColumn<casa::String> roFilterSelection_p;
    casa::ROScalarColumn<casa::Double> roClockFrequency_p;
    casa::ROArrayColumn<casa::String>  roTarget_p;
    casa::ROScalarColumn<casa::String> roSystemVersion_p;
    casa::ROScalarColumn<casa::String> roPipelineName_p;
    casa::ROScalarColumn<casa::String> roPipelineVersion_p;
    casa::ROScalarColumn<casa::String> roFilename_p;
    casa::ROScalarColumn<casa::String> roFiletype_p;
    casa::ROScalarColumn<casa::Double> roFiledate_p;
    //# Access to Quantum columns
    casa::ROScalarQuantColumn<casa::Double> roObservationStartQuant_p;
    casa::ROScalarQuantColumn<casa::Double> roObservationEndQuant_p;
    casa::ROScalarQuantColumn<casa::Double> roObservationFrequencyMaxQuant_p;
    casa::ROScalarQuantColumn<casa::Double> roObservationFrequencyMinQuant_p;
    casa::ROScalarQuantColumn<casa::Double> roObservationFrequencyCenterQuant_p;
    casa::ROScalarQuantColumn<casa::Double> roClockFrequencyQuant_p;
    casa::ROScalarQuantColumn<casa::Double> roFiledateQuant_p;
    //# Access to Measure columns
    casa::ROScalarMeasColumn<casa::MEpoch> roObservationStartMeas_p;
    casa::ROScalarMeasColumn<casa::MEpoch> roObservationEndMeas_p;
    casa::ROScalarMeasColumn<casa::MEpoch> roFiledateMeas_p;
    //# required columns
    casa::ScalarColumn<casa::String> rwProjectTitle_p;
    casa::ScalarColumn<casa::String> rwProjectPI_p;
    casa::ArrayColumn<casa::String>  rwProjectCoI_p;
    casa::ScalarColumn<casa::String> rwProjectContact_p;
    casa::ScalarColumn<casa::String> rwObservationId_p;
    casa::ScalarColumn<casa::Double> rwObservationStart_p;
    casa::ScalarColumn<casa::Double> rwObservationEnd_p;
    casa::ScalarColumn<casa::Double> rwObservationFrequencyMax_p;
    casa::ScalarColumn<casa::Double> rwObservationFrequencyMin_p;
    casa::ScalarColumn<casa::Double> rwObservationFrequencyCenter_p;
    casa::ScalarColumn<casa::Int>    rwSubArrayPointing_p;
    casa::ScalarColumn<casa::Int>    rwNofBitsPerSample_p;
    casa::ScalarColumn<casa::String> rwAntennaSet_p;
    casa::ScalarColumn<casa::String> rwFilterSelection_p;
    casa::ScalarColumn<casa::Double> rwClockFrequency_p;
    casa::ArrayColumn<casa::String>  rwTarget_p;
    casa::ScalarColumn<casa::String> rwSystemVersion_p;
    casa::ScalarColumn<casa::String> rwPipelineName_p;
    casa::ScalarColumn<casa::String> rwPipelineVersion_p;
    casa::ScalarColumn<casa::String> rwFilename_p;
    casa::ScalarColumn<casa::String> rwFiletype_p;
    casa::ScalarColumn<casa::Double> rwFiledate_p;
    //# Access to Quantum columns
    casa::ScalarQuantColumn<casa::Double> rwObservationStartQuant_p;
    casa::ScalarQuantColumn<casa::Double> rwObservationEndQuant_p;
    casa::ScalarQuantColumn<casa::Double> rwObservationFrequencyMaxQuant_p;
    casa::ScalarQuantColumn<casa::Double> rwObservationFrequencyMinQuant_p;
    casa::ScalarQuantColumn<casa::Double> rwObservationFrequencyCenterQuant_p;
    casa::ScalarQuantColumn<casa::Double> rwClockFrequencyQuant_p;
    casa::ScalarQuantColumn<casa::Double> rwFiledateQuant_p;
    //# Access to Measure columns
    casa::ScalarMeasColumn<casa::MEpoch> rwObservationStartMeas_p;
    casa::ScalarMeasColumn<casa::MEpoch> rwObservationEndMeas_p;
    casa::ScalarMeasColumn<casa::MEpoch> rwFiledateMeas_p;
  };

} //# end namespace

#endif
