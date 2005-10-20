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
#include <Transport/TH_Mem.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Ethernet.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <WH_Signal.h>
#include <WH_FakeStation.h>
#include <WH_Strip.h>
#include <WH_Wrap.h>

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

  int NRSP = itsParamSet.getInt32("Input.NRSP");
  int WH_DH_NameSize = 40;
  char WH_DH_Name[WH_DH_NameSize];
  bool useEth = itsParamSet.getBool("Generator.UseEth");
  vector<string> outFileNames = itsParamSet.getStringVector("Generator.OutputFiles");
  vector<string> interfaces = itsParamSet.getStringVector("Generator.Interfaces");
  vector<string> dstMacs = itsParamSet.getStringVector("Input.DestinationMacs");
  vector<string> srcMacs = itsParamSet.getStringVector("Input.SourceMacs");
  vector<int32> stationIds = itsParamSet.getInt32Vector("Generator.StationIds");
  vector<int32> delays = itsParamSet.getInt32Vector("Generator.StationDelays");
  
  snprintf(WH_DH_Name, WH_DH_NameSize, "Signal");

  WH_Signal::SignalType st;
  string signalType = itsParamSet.getString("Generator.SignalType");
  if (signalType == "ZERO") {
    st = WH_Signal::ZERO;
  } else if (signalType == "RANDOM") {
    st = WH_Signal::RANDOM;
  } else if (signalType == "PATTERN") {
    st = WH_Signal::PATTERN;
  } else {
    ASSERTSTR(false, "Signaltype unknown. Generator.SignalType should be one of {ZERO, PATTERN, RANDOM}");
  }  
  
  itsWHs.push_back(new WH_Signal(WH_DH_Name, 
				 NRSP,
				 itsParamSet,
				 st));
  Step* itsSignalStep = new Step(itsWHs.back(), WH_DH_Name);
  itsSteps.push_back(itsSignalStep);
  comp.addBlock(itsSignalStep);

  for (int s=0; s<NRSP; s++) {
    snprintf(WH_DH_Name, WH_DH_NameSize, "FakeStation_%d_of_%d", s, NRSP);
    if (useEth) {
      // cout<<"interface: "<<interfaces[s]<<" remote: "<<dstMacs[s]<<" own: "<<srcMacs[s]<<endl;
      itsTHs.push_back(new TH_Ethernet(interfaces[s], 
				       dstMacs[s],
				       srcMacs[s]));
    } else {
      itsTHs.push_back(new TH_File(outFileNames[s], TH_File::Write));
    }
    ASSERTSTR(itsTHs.back()->init(), "Could not init TransportHolder");
    itsWHs.push_back(new WH_FakeStation(WH_DH_Name,
					itsParamSet,
					stationIds[s],
					delays[s]));
    itsSteps.push_back(new Step(itsWHs.back(), WH_DH_Name));
    // share input and output DH, no cyclic buffer
    itsSteps.back()->setInBufferingProperties(0, true, true);

    comp.addBlock(itsSteps.back());
    itsSteps.back()->connect(0, itsSignalStep, s, 1,
			     new TH_Mem(),
			     false);

    snprintf(WH_DH_Name, WH_DH_NameSize, "Strip_%d_of_%d", s, NRSP);
    itsWHs.push_back(new WH_Strip(WH_DH_Name,
				  *itsTHs.back(),
				  itsParamSet));
    itsSteps.push_back(new Step(itsWHs.back(), WH_DH_Name));
    comp.addBlock(itsSteps.back());
    itsSteps.back()->connect(0, itsSteps[itsSteps.size()-2], 0, 1,
			     new TH_Mem(),
			     false);
  };

#ifdef HAVE_MPI
  // TODO How do we want to distribute the steps across the nodes?
  //  ASSERTSTR (lastFreeNode == TH_MPI::getNumberOfNodes(), lastFreeNode << " nodes needed, "<<TH_MPI::getNumberOfNodes()<<" available");
#endif

  LOG_TRACE_FLOW_STR("Finished defineGenerator()");
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


