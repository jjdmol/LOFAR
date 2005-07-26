//#  AH_FakeStation.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//
/////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/lofar_iostream.h>

#include <APS/ParameterSet.h>

#include <AH_FakeStation.h>

// Transporters
#include <Transport/TH_File.h>
#include <Transport/TH_Ethernet.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <TFC_Generator/WH_FakeStation.h>

using namespace LOFAR;

AH_FakeStation::AH_FakeStation() 
{
}

AH_FakeStation::~AH_FakeStation() 
{
  this->undefine();
}

void AH_FakeStation::undefine() {
  vector<WorkHolder*>::iterator wit = itsWHs.begin();
  for (; wit!=itsWHs.end(); wit++) {
    delete *wit;
  }
  itsWHs.clear();
  vector<Step*>::iterator sit = itsSteps.begin();
  for (; sit!=itsSteps.end(); sit++) {
    delete *sit;
  }
  itsSteps.clear();
  vector<TransportHolder*>::iterator tit = itsTHs.begin();
  for (; tit!=itsTHs.end(); tit++) {
    delete *tit;
  }
  itsTHs.clear();
}  

void AH_FakeStation::define(const LOFAR::KeyValueMap&) {
  LOG_TRACE_FLOW_STR("Start of AH_FakeStation::define()");
  undefine();
    
  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

  LOG_TRACE_FLOW_STR("Create output side interface stubs");
  // todo: define this input interface; although there are no
  //       connection involved, we do have to define the port/IP numbering schemes

  int NRSP = itsParamSet.getInt32("NRSP");
  int WH_DH_NameSize = 40;
  char WH_DH_Name[WH_DH_NameSize];
  bool useEth = itsParamSet.getBool("Generator.UseEth");
  vector<string> interfaces = itsParamSet.getStringVector("Generator.Interfaces");
  vector<string> remMacs = itsParamSet.getStringVector("Generator.RemMacs");
  vector<string> ownMacs = itsParamSet.getStringVector("Generator.OwnMacs");
  vector<int32> stationIds = itsParamSet.getInt32Vector("Generator.StationIds");
  
  for (int s=0; s<NRSP; s++) {
    snprintf(WH_DH_Name, WH_DH_NameSize, "FakeStation_%d_of_%d", s, NRSP);
    if (useEth) {
      // cout<<"interface: "<<interfaces[s]<<" remote: "<<remMacs[s]<<" own: "<<ownMacs[s]<<endl;
      itsTHs.push_back(new TH_Ethernet(interfaces[s], 
				       remMacs[s],
				       ownMacs[s]));
    } else {
      itsTHs.push_back(new TH_File("Generator.out", TH_File::Write));
    }
    ASSERTSTR(itsTHs.back()->init(), "Could not init TransportHolder");
    itsWHs.push_back(new WH_FakeStation(WH_DH_Name,
					itsParamSet,
					*itsTHs.back(),
					stationIds[s]));
    itsSteps.push_back(new Step(itsWHs.back(),WH_DH_Name,false));
    comp.addBlock(itsSteps.back());
  };
  
  LOG_TRACE_FLOW_STR("Finished define()");
}



void AH_FakeStation::prerun() {
  getComposite().preprocess();
}
    
void AH_FakeStation::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_FakeStation::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_FakeStation::run() "  );
}

void AH_FakeStation::dump() const {
  LOG_TRACE_FLOW_STR("AH_FakeStation::dump() not implemented"  );
}

void AH_FakeStation::quit() {
}


