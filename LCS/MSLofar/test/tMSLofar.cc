//# tMSLofar.cc: Test program for class MSLofar
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

#include <lofar_config.h>

#include <MSLofar/MSLofar.h>
#include <MSLofar/MSLofarAntennaColumns.h>
#include <MSLofar/MSLofarFieldColumns.h>
#include <MSLofar/MSLofarObsColumns.h>
#include <MSLofar/MSStationColumns.h>
#include <MSLofar/MSAntennaFieldColumns.h>
#include <MSLofar/MSElementFailureColumns.h>
#include <Common/LofarLogger.h>

#include <tables/Tables/SetupNewTab.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/OS/Timer.h>

using namespace LOFAR;
using namespace casa;
using namespace std;

void createMS (MDirection::Types dirRef)
{
  Timer timer;
  TableDesc td(MSLofar::requiredTableDesc());
  SetupNewTable setup("tMSLofar_tmp.ms", td, Table::New);
  MSLofar ms(setup);
  ms.createDefaultSubtables(Table::New);
  MSLofarFieldColumns fieldCols(ms.field());
  fieldCols.setDirectionRef (dirRef);
  ms.flush();
  timer.show("create");
}

void openMS()
{
  Timer timer;
  MSLofar ms("tMSLofar_tmp.ms");
  MSLofarAntenna ant(ms.antenna());
  MSLofarObservation obs(ms.observation());
  MSStation stat(ms.station());
  MSAntennaField antFld(ms.antennaField());
  MSElementFailure fail(ms.elementFailure());
  timer.show("open  ");
}

void fillMS()
{
  MSLofar ms("tMSLofar_tmp.ms", Table::Update);
  MSLofarAntennaColumns ant(ms.antenna());
  MSLofarFieldColumns fld(ms.field());
  MSLofarObservationColumns obs(ms.observation());
  MSStationColumns stat(ms.station());
  MSAntennaFieldColumns antFld(ms.antennaField());
  MSElementFailureColumns fail(ms.elementFailure());
  // Put data into LOFAR specific subtables.
  ms.antenna().addRow();
  ant.name().put (0, "ant1");
  ant.stationId().put (0, 1);
  ant.type().put (0, "type1");
  ant.mount().put (0, "mount1");
  ant.position().put (0, Vector<double>(3,1.));
  ant.offset().put (0, Vector<double>(3,2.));
  ant.dishDiameter().put (0, 25.);
  ant.phaseReference().put (0, Vector<double>(3,-1.));

  ms.field().addRow();
  fld.name().put (0, "fld1");
  fld.code().put (0, "cd");
  fld.time().put (0, 34.);
  fld.numPoly().put (0, 0);
  fld.phaseDir().put (0, Matrix<double>(2,1,1.1));
  fld.delayDir().put (0, Matrix<double>(2,1,1.2));
  fld.referenceDir().put (0, Matrix<double>(2,1,1.3));
  fld.tileBeamDir().put (0, Matrix<double>(2,1,1.5));
  fld.sourceId().put (0, -1);
  fld.flagRow().put (0, False);

  ms.observation().addRow();
  obs.telescopeName().put (0, "LOFAR");
  obs.timeRange().put (0, Vector<double>(2,10.));
  obs.observer().put (0, "someName");
  obs.log().put (0, Vector<String>());
  obs.scheduleType().put (0, "sched");
  obs.schedule().put (0, Vector<String>(1,"s"));
  obs.project().put (0, "pr");
  obs.projectTitle().put (0, "desc");
  obs.projectPI().put (0, "pi");
  obs.projectCoI().put (0, Vector<String>(2, "dd"));
  obs.observationId().put (0, "123");
  obs.observationStart().put (0, 24*3600);
  obs.observationEnd().put (0, 2*24*3600);
  obs.observationFrequencyMax().put (0, 60);
  obs.observationFrequencyMin().put (0, 20);
  obs.observationFrequencyCenter().put (0, 40);
  obs.subArrayPointing().put (0, 3);
  obs.nofBitsPerSample().put (0, 16);
  obs.antennaSet().put (0, "HBA_DUAL");
  obs.filterSelection().put (0, "sel");
  obs.clockFrequency().put (0, 160);
  obs.target().put (0, Vector<String>(10, "tar"));
  obs.systemVersion().put (0, "3.1.5");
  obs.pipelineName().put (0, "SIP");
  obs.pipelineVersion().put (0, "1.0");
  obs.filename().put (0, "thisFile");
  obs.filetype().put (0, "uv");
  obs.filedate().put (0, 10*24*3600);
  obs.releaseDate().put (0, 100*24*3600);

  ms.station().addRow();

  ms.antennaField().addRow();

  ms.elementFailure().addRow();
}

void checkMS (const MSLofar& ms)
{
  ROMSLofarAntennaColumns ant(ms.antenna());
  ROMSLofarFieldColumns fld(ms.field());
  ROMSLofarObservationColumns obs(ms.observation());
  ROMSStationColumns stat(ms.station());
  ROMSAntennaFieldColumns antFld(ms.antennaField());
  ROMSElementFailureColumns fail(ms.elementFailure());
  // Check data in LOFAR specific subtables.
  ASSERT (ant.name()(0) == "ant1");
  ASSERT (ant.stationId()(0) == 1);
  ASSERT (ant.type()(0) == "type1");
  ASSERT (ant.mount()(0) == "mount1");
  ASSERT (allEQ(ant.position()(0), Vector<double>(3,1.)));
  ASSERT (allEQ(ant.offset()(0), Vector<double>(3,2.)));
  ASSERT (ant.dishDiameter()(0) == 25.);
  ASSERT (allEQ(ant.phaseReference()(0), Vector<double>(3,-1.)));
  Vector<Quantity> phRef = ant.phaseReferenceQuant()(0,"deg");
  ASSERT (phRef.size()==3 &&
          near(phRef[0].getValue(), -1*180/C::pi) &&
          near(phRef[1].getValue(), -1*180/C::pi) &&
          near(phRef[2].getValue(), -1*180/C::pi));
  MPosition phRefPos = ant.phaseReferenceMeas()(0);
  phRefPos.print(cout);
  cout<<endl;
  
  ASSERT (fld.name()(0) == "fld1");
  ASSERT (fld.code()(0) == "cd");
  ASSERT (fld.time()(0) == 34.);
  ASSERT (fld.numPoly()(0) == 0);
  ASSERT (allNear (fld.phaseDir()(0), Matrix<double>(2,1,1.1), 1e-7));
  ASSERT (allNear (fld.delayDir()(0), Matrix<double>(2,1,1.2), 1e-7));
  ASSERT (allNear (fld.referenceDir()(0), Matrix<double>(2,1,1.3), 1e-7));
  ASSERT (allNear (fld.tileBeamDir()(0), Matrix<double>(2,1,1.5), 1e-7));
  ASSERT (fld.sourceId()(0) == -1);
  ASSERT (fld.flagRow()(0) == False);
  MDirection tileBeamDir = fld.tileBeamDirMeasCol()(0);
  tileBeamDir.print(cout);
  cout << ' ' << tileBeamDir.getRefString() << endl;

  ASSERT (obs.telescopeName()(0) == "LOFAR");
  ASSERT (allEQ(obs.timeRange()(0), Vector<double>(2,10.)));
  ASSERT (obs.observer()(0) == "someName");
  ASSERT (allEQ(obs.log()(0), Vector<String>()));
  ASSERT (obs.scheduleType()(0) == "sched");
  ASSERT (allEQ(obs.schedule()(0), Vector<String>(1,"s")));
  ASSERT (obs.project()(0) == "pr");
  ASSERT (obs.projectTitle()(0) == "desc");
  ASSERT (obs.projectPI()(0) == "pi");
  ASSERT (allEQ(obs.projectCoI()(0), Vector<String>(2,"dd")));
  ASSERT (obs.observationId()(0) == "123");
  ASSERT (obs.observationStart()(0) == 24*3600);
  ASSERT (obs.observationEnd()(0) == 2*24*3600);
  ASSERT (obs.observationFrequencyMax()(0) == 60);
  ASSERT (obs.observationFrequencyMin()(0) == 20);
  ASSERT (obs.observationFrequencyCenter()(0) == 40);
  ASSERT (obs.subArrayPointing()(0) == 3);
  ASSERT (obs.nofBitsPerSample()(0) == 16);
  ASSERT (obs.antennaSet()(0) == "HBA_DUAL");
  ASSERT (obs.filterSelection()(0) == "sel");
  ASSERT (obs.clockFrequency()(0) == 160);
  ASSERT (allEQ(obs.target()(0), Vector<String>(10,"tar")));
  ASSERT (obs.systemVersion()(0) == "3.1.5");
  ASSERT (obs.pipelineName()(0) == "SIP");
  ASSERT (obs.pipelineVersion()(0) == "1.0");
  ASSERT (obs.filename()(0) == "thisFile");
  ASSERT (obs.filetype()(0) == "uv");
  ASSERT (obs.filedate()(0) == 10*24*3600);
  ASSERT (obs.releaseDate()(0) == 100*24*3600);
  ASSERT (near(obs.observationStartQuant()(0,"d").getValue(), 1.));
  ASSERT (near(obs.observationEndQuant()(0,"d").getValue(), 2.));
  ASSERT (near(obs.observationFrequencyMaxQuant()(0,"Hz").getValue(), 60e6));
  ASSERT (near(obs.observationFrequencyMinQuant()(0,"Hz").getValue(), 20e6));
  ASSERT (near(obs.observationFrequencyCenterQuant()(0,"Hz").getValue(), 40e6));
  ASSERT (near(obs.clockFrequencyQuant()(0,"GHz").getValue(), 0.16));
  ASSERT (near(obs.filedateQuant()(0,"d").getValue(), 10.));
  obs.observationStartMeas()(0).print(cout);
  cout << ' ';
  obs.observationEndMeas()(0).print(cout);
  cout << ' ';
  obs.filedateMeas()(0).print(cout);
  cout<<endl;
}

int main()
{
  try {
    // Create for a J2000 direction.
    createMS (MDirection::J2000);
    openMS();
    fillMS();
    checkMS (MSLofar("tMSLofar_tmp.ms"));
    // Create for a SUN direction.
    createMS (MDirection::SUN);
    openMS();
    fillMS();
    checkMS (MSLofar("tMSLofar_tmp.ms"));
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
