//# tOutputThread.cc: Test FastFileStream class
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include <lofar_config.h>

#include <OutputProc/OutputThread.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Exceptions.h>
#include <OutputProc/MSWriterNull.h>

#include <iostream>
#include <UnitTest++.h>

using namespace std;
using namespace LOFAR::Cobalt;


TEST(testCorrelatorOutputThreadThrowsStorageException)
{
  cout << "testCorrelatorOutputThreadThrowsStorageException" << endl;
  Parset par;
  par.add("Observation.startTime","2011-03-22 18:16:00");
  par.add("OLAP.CNProc.integrationSteps","256");
  par.add("OLAP.IONProc.integrationSteps", "4");
  par.add("OLAP.correctBandPass", "F");
  par.add("Observation.nrBitsPerSample", "8");
  par.add("OLAP.CNProc.nrPPFTaps", "16");
  par.add("Observation.VirtualInstrument.stationList", "[CS002]");
  par.add("Observation.antennaSet", "HBA_DUAL");
  par.add("Observation.nrBeams", "1");
  par.add("Observation.Beam[0].subbandList", "[300..301]");
  par.add("Observation.rspBoardList", "[0, 0]");
  par.add("Observation.rspSlotList", "[0, 1]");
  par.add("OLAP.CNProc_CoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_CoherentStokes.which", "I");
  par.add("OLAP.CNProc_IncoherentStokes.which", "I");
  par.add("OLAP.delayCompensation", "F");
  par.add("Observation.nrPolarisations", "2");
  par.add("Observation.channelsPerSubband", "1");
  par.add("Observation.sampleClock", "200");
  par.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  par.add("Observation.DataProducts.Output_Correlated.filenames", "[2*L173014_SAP000_SB000_uv.MS]");
  par.add("Observation.DataProducts.Output_Correlated.locations", "[.:./, .:./]");
  // MIssing key: This will lead to an exception triggering the new code 
  //par.add("PIC.Core.CS002HBA0.phaseCenter", "0.0")

  par.updateSettings();
  Pool<StreamableData> outputPool("testCorrelatorOutputThreadThrowsStorageException::outputPool");
  try
  {
    SubbandOutputThread SubbandOutputThread(par, 
                (unsigned) 0, outputPool, "Log prefix", "./");

    SubbandOutputThread.createMS();
  }
  catch (StorageException &ex)  // Catch the correct exception
  {
    cout << "Got the correct exeption. " << endl;
    cout << ex.what() << endl;

  }
  //else  Let the exception fall tru
  
  cout << "Succes" << endl;
}


/* **************************
 * wrapper class for SubbandOutputThread allows access to the MSWriter type

*/
 class OutputThreadWrapper: public SubbandOutputThread
{
  public: 
    OutputThreadWrapper(const Parset &parset, unsigned streamNr, Pool<StreamableData> &outputPool, const std::string &logPrefix, const std::string &targetDirectory = "")
  :
  SubbandOutputThread(parset, streamNr,
  outputPool, logPrefix,
  targetDirectory)
  {
  }

    MSWriter*   getMSWriter()
  {
    return itsWriter.get();
  }

};

TEST(testCorrelatorOutputThreadRealtimeThrowsNoException)
{
  cout << "testCorrelatorOutputThreadRealtimeThrowsNoException" << endl;
  Parset par;
  par.add("Observation.startTime", "2011-03-22 18:16:00");
  par.add("OLAP.CNProc.integrationSteps", "256");
  par.add("OLAP.IONProc.integrationSteps", "4");
  par.add("OLAP.correctBandPass", "F");
  par.add("Observation.nrBitsPerSample", "8");
  par.add("OLAP.CNProc.nrPPFTaps", "16");
  par.add("Observation.VirtualInstrument.stationList", "[CS002]");
  par.add("Observation.antennaSet", "HBA_DUAL");
  par.add("Observation.nrBeams", "1");
  par.add("Observation.Beam[0].subbandList", "[300..301]");
  par.add("Observation.rspBoardList", "[0, 0]");
  par.add("Observation.rspSlotList", "[0, 1]");
  par.add("OLAP.CNProc_CoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_CoherentStokes.which", "I");
  par.add("OLAP.CNProc_IncoherentStokes.which", "I");
  par.add("OLAP.delayCompensation", "F");
  par.add("Observation.nrPolarisations", "2");
  par.add("Observation.channelsPerSubband", "1");
  par.add("Observation.sampleClock", "200");
  par.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  par.add("Observation.DataProducts.Output_Correlated.filenames", "[2*L173014_SAP000_SB000_uv.MS]");
  par.add("Observation.DataProducts.Output_Correlated.locations", "[.:./, .:./]");
  // MIssing key: This will lead to an exception triggering the new code 
  //par.add("PIC.Core.CS002HBA0.phaseCenter", "0.0")
  par.add("Cobalt.realTime", "true");
    par.updateSettings();
  Pool<StreamableData> outputPool("testCorrelatorOutputThreadRealtimeThrowsNoException::outputPool");


  // We have a realtime system. We should not throw execptions
  OutputThreadWrapper SubbandOutputThread(par,
      (unsigned)0, outputPool, "Log prefix", "./");

  SubbandOutputThread.createMS();

  // asure null writer
  ASSERT( (dynamic_cast<MSWriterNull*> (SubbandOutputThread.getMSWriter()) != 0));

  cout << "Succes" << endl;
}

/***************************************************
 * test for the breamformer writers
 */

TEST(testBeamformerOutputThreadThrowsStorageException)
{
  cout << "testCorrelatorOutputThreadThrowsStorageException" << endl;
  Parset par;
  par.add("Observation.startTime", "2011-03-22 18:16:00");
  par.add("OLAP.CNProc.integrationSteps", "256");
  par.add("OLAP.IONProc.integrationSteps", "4");
  par.add("OLAP.correctBandPass", "F");
  par.add("Observation.nrBitsPerSample", "8");
  par.add("OLAP.CNProc.nrPPFTaps", "16");
  par.add("Observation.VirtualInstrument.stationList", "[CS002]");
  par.add("Observation.antennaSet", "HBA_DUAL");
  par.add("Observation.nrBeams", "1");
  par.add("Observation.Beam[0].subbandList", "[300..301]");
  par.add("Observation.rspBoardList", "[0, 0]");
  par.add("Observation.rspSlotList", "[0, 1]");
  par.add("OLAP.CNProc_CoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_CoherentStokes.which", "I");
  par.add("OLAP.CNProc_IncoherentStokes.which", "I");
  par.add("OLAP.delayCompensation", "F");
  par.add("Observation.nrPolarisations", "2");
  par.add("Observation.channelsPerSubband", "1");
  par.add("Observation.sampleClock", "200");
  par.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  par.add("Observation.DataProducts.Output_Correlated.filenames", "[2*L173014_SAP000_SB000_uv.MS]");
  par.add("Observation.DataProducts.Output_Correlated.locations", "[.:./, .:./]");
  // MIssing key: This will lead to an exception triggering the new code 
  //par.add("PIC.Core.CS002HBA0.phaseCenter", "0.0")

  par.updateSettings();
  Pool<StreamableData> outputPool("testBeamformerOutputThreadThrowsStorageException::outputPool");
  try
  {
    SubbandOutputThread SubbandOutputThread(par,
      (unsigned)0, outputPool, "Log prefix", "./");

    SubbandOutputThread.createMS();
  }
  catch (StorageException &ex)  // Catch the correct exception
  {
    cout << "Got the correct exeption. " << endl;
    cout << ex.what() << endl;

  }
  //else  Let the exception fall tru

  cout << "Succes" << endl;
}


/* **************************
* wrapper class for SubbandOutputThread allows access to the MSWriter type

*/
class TABOutputThreadWrapper : public TABOutputThread
{
public:
  TABOutputThreadWrapper(const Parset &parset, unsigned streamNr,
    Pool<TABTranspose::BeamformedData> &outputPool, const std::string &logPrefix,
    const std::string &targetDirectory)
    :
    TABOutputThread(parset, streamNr,
    outputPool, logPrefix,
    targetDirectory)
  {
  }

  MSWriter*   getMSWriter()
  {
    return itsWriter.get();
  }

};

TEST(testBeamformerOutputThreadRealtimeThrowsNoException)
{
  cout << "testCorrelatorOutputThreadRealtimeThrowsNoException" << endl;
  Parset par;
  par.add("Observation.startTime", "2011-03-22 18:16:00");
  par.add("OLAP.CNProc.integrationSteps", "256");
  par.add("OLAP.IONProc.integrationSteps", "4");
  par.add("OLAP.correctBandPass", "F");
  par.add("Observation.nrBitsPerSample", "8");
  par.add("OLAP.CNProc.nrPPFTaps", "16");
  par.add("Observation.VirtualInstrument.stationList", "[CS002]");
  par.add("Observation.antennaSet", "HBA_DUAL");
  par.add("Observation.nrBeams", "1");
  par.add("Observation.Beam[0].subbandList", "[300]");
  par.add("Observation.Beam[0].nrTiedArrayBeams", "1");
  par.add("Observation.Beam[1].TiedArrayBeam[0].absoluteAngle1", "0");
  par.add("Observation.Beam[1].TiedArrayBeam[0].absoluteAngle2", "0");
  par.add("Observation.Beam[1].TiedArrayBeam[0].coherent", "T");
  par.add("Observation.rspBoardList", "[0, 0]");
  par.add("Observation.rspSlotList", "[0, 1]");
  par.add("OLAP.CNProc_CoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor", "1");
  par.add("OLAP.CNProc_CoherentStokes.which", "I");
  par.add("OLAP.CNProc_IncoherentStokes.which", "I");
  par.add("OLAP.delayCompensation", "F");
  par.add("Observation.nrPolarisations", "2");
  par.add("Observation.channelsPerSubband", "1");
  par.add("Observation.sampleClock", "200");
  par.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
  par.add("Observation.DataProducts.Output_CoherentStokes.filenames", "[tab1.raw]");
  par.add("Observation.DataProducts.Output_CoherentStokes.locations", "[1 * :/NonExisting]");
  // MIssing key: This will lead to an exception triggering the new code 
  //par.add("PIC.Core.CS002HBA0.phaseCenter", "0.0")
  par.add("Cobalt.realTime", "true");
  par.updateSettings();
  Pool<TABTranspose::BeamformedData> outputPool("testBeamformerOutputThreadRealtimeThrowsNoException");


  // We have a realtime system. We should not throw execptions
  TABOutputThreadWrapper BeamOutputThread(par,
    (unsigned)0, outputPool, "Log prefix", "/NonExisting");

  BeamOutputThread.createMS();

  //// asure null writer
  ASSERT((dynamic_cast<MSWriterNull*> (BeamOutputThread.getMSWriter()) != 0));
  
  cout << "Succes" << endl;
}


int main()
{

  return UnitTest::RunAllTests() > 0;
  return 0;
}

