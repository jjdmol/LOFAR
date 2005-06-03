//#  WH_Plot2MAC.cc: Output workholder for BG Correlator application
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$


#include <stdio.h>           // for sprintf
#include <sys/types.h>       // for file output
#include <fcntl.h>           // for file output
#include <math.h>            // for sqrt()

// General includes
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>

// Application specific includes
#include <WH_Plot2MAC.h>
#include <DH_Vis.h>


#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>

using namespace LOFAR;


WH_Plot2MAC::WH_Plot2MAC(const string& name, const KeyValueMap& kvm)
  : WorkHolder(kvm.getInt("NoWH_Correlator",7)/kvm.getInt("NoWH_Dump", 2), 0, name, "WH_Plot2MAC"),
    itsBandwidth(0),
    itsKvm(kvm),
    itsPlotPS(0)
{
  itsNelements      = kvm.getInt("NoWH_RSP", 2);
  itsNchannels      = kvm.getInt("NoRSPBeamlets", 92)/kvm.getInt("NoWH_Correlator", 92);
  itsNpolarisations = kvm.getInt("polarisations", 2);
  
  char DHName[20];
  for (int i=0; i<itsNinputs; i++) {
    sprintf(DHName, "input_%3d_of_%3d", i, itsNinputs);
    getDataManager().addInDataHolder(i, new DH_Vis(DHName, 
						   itsNelements, 
						   itsNchannels, 
						   itsNpolarisations));
  }

  itsLastTime.tv_sec = 0;
  itsLastTime.tv_usec = 0;
  // From now on the number of polarisations is squared because we
  // are behind the correlator. The DataHolder that is added above
  // does the multiplication in its own constructor
  itsNpolarisations *= itsNpolarisations;
}

WH_Plot2MAC::~WH_Plot2MAC() {
  delete itsPlotPS;
}

WorkHolder* WH_Plot2MAC::construct (const string& name, 
				const KeyValueMap& kvm)
{
  return new WH_Plot2MAC(name, kvm);
}

WH_Plot2MAC* WH_Plot2MAC::make(const string& name) {
  return new WH_Plot2MAC(name, itsKvm); 
}

void WH_Plot2MAC::preprocess() {
  LOG_TRACE_FLOW("WH_Plot2MAC enabling PropertySet");
  itsPlotPS = new GCF::CEPPMLlight::CEPPropertySet("CEP_SCD", "TStationCorrelator", GCF::Common::PS_CAT_PERMANENT);
  itsPlotPS->enable();
  LOG_TRACE_FLOW("WH_Plot2MAC PropertySet enabled");
}

void WH_Plot2MAC::process() {

  struct timeval newTime;
  long recSize = 0;
  long written = 0;
  long totalWritten = 0;
  long totalWrittenKB = 0;
  DH_Vis* dhp;

  // Right now we write to disk per frequency channel
  // The file consists of a multidimensional array: channels*station*station*npolariations
  // Every block of stations*stations holds the real values in the upper triangle and the 
  // imaginary values in the lower triangle. 

  // This will probably give some performance problems. If it does we can do the following improvements:
  // 1 Put the real and imaginairy values together and make the dimensions of the array dynamic. This way no
  //   extra memory is used and we can read the complex numbers as a whole.
  // 2 Make the format at the output of the correlator the same as in the file or vice versa. Then we can read
  //   and write the data in very large blocks.

  // We need some way to store metadata. Maybe in another file (or a piece of paper :)

  DH_Vis::BufferPrimitive freqBlock[itsNelements*itsNelements*itsNpolarisations];

  LOG_TRACE_FLOW("WH_Plot2MAC analyzing input");
  
  stringstream autoCor1, autoCor2, crossCor;
  for (int i=0; i<itsNinputs; i++) {
    dhp = (DH_Vis*)getDataManager().getInHolder(i);
    recSize += dhp->getBufSize()*sizeof(DH_Vis::BufferType);
    for (int channel=0; channel<itsNchannels; channel++) {

      // i'm not sure what data we want
      double totalP = abs(*dhp->getBufferElement(0, 0, channel, 0));
      autoCor1 << i*itsNchannels + channel << " " << totalP << endl; // autoCor1 is in dhp->getBufferElement(0, 0, channel, < 0 | 1 | 2 | 3 > )
      totalP = abs(*dhp->getBufferElement(1, 1, channel, 0));
      autoCor2 << i*itsNchannels + channel << " " << totalP << endl; // autoCor2 is in dhp->getBufferElement(1, 1, channel, < 0 | 1 | 2 | 3 > )
      totalP = abs(*dhp->getBufferElement(1, 0, channel, 0));
      crossCor << i*itsNchannels + channel << " " << totalP << endl; // crossCor is in dhp->getBufferElement(1, 0, channel, < 0 | 1 | 2 | 3 > )

    }
  }

  LOG_TRACE_FLOW("WH_Plot2MAC setting properties");
  
    cout<<"noInputs: "<<itsNinputs<<"    noBeamlets: "<<itsNchannels<<endl;
    cout<<"autocorr1"<<endl<<"====="<<endl<<autoCor1.str()<<endl;
    cout<<"autocorr2"<<endl<<"====="<<endl<<autoCor2.str()<<endl;
    cout<<"crosscorr"<<endl<<"====="<<endl<<crossCor.str()<<endl;
  
  cout<<"sending update to MAC"<<endl;

  (*itsPlotPS)["autocorr1"].setValue(GCF::Common::GCFPVString(autoCor1.str()));
  (*itsPlotPS)["autocorr2"].setValue(GCF::Common::GCFPVString(autoCor2.str()));
  (*itsPlotPS)["crosscorr"].setValue(GCF::Common::GCFPVString(crossCor.str()));

  LOG_TRACE_FLOW("WH_Plot2MAC properties set");

  memcpy(&itsLastTime, &newTime, sizeof(struct timeval));
}

void WH_Plot2MAC::postProcess() {
}
