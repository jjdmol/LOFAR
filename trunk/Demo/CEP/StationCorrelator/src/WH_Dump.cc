//#  WH_Dump.cc: Output workholder for BG Correlator application
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

// General includes
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>

// Application specific includes
#include <WH_Dump.h>
#include <DH_Vis.h>

using namespace LOFAR;


WH_Dump::WH_Dump(const string& name, const KeyValueMap& kvm)
  : WorkHolder(kvm.getInt("NoWH_Correlator",7)/kvm.getInt("NoWH_Dump", 2), 0, name, "WH_Dump"),
    itsOutputFile(0),
    itsBandwidth(0),
    itsKvm(kvm)
{
  itsOutputFileName = kvm.getString("outFileName", "StatCor.out");
  itsNelements      = kvm.getInt("stations", 2);
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

WH_Dump::~WH_Dump() {
}

WorkHolder* WH_Dump::construct (const string& name, 
				const KeyValueMap& kvm)
{
  return new WH_Dump(name, kvm);
}

WH_Dump* WH_Dump::make(const string& name) {
  return new WH_Dump(name, itsKvm); 
}

void WH_Dump::preprocess() {
  int openFlags = O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE;
  mode_t permissions = S_IREAD | S_IWRITE | S_IRGRP | S_IROTH;
  itsOutputFile = open(itsOutputFileName.c_str(), openFlags, permissions);
  ASSERTSTR(itsOutputFile != -1, "Could not open outputfile " << itsOutputFileName);
}

void WH_Dump::process() {

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
  
  for (int i=0; i<itsNinputs; i++) {
    dhp = (DH_Vis*)getDataManager().getInHolder(i);
    recSize += dhp->getBufSize()*sizeof(DH_Vis::BufferType);
    for (int channel=0; channel<itsNchannels; channel++) {
      for (int el1=0; el1<itsNelements; el1++){
	for (int el2=0; el2<=el1; el2++){
	  int offsetRe = el1 * (itsNelements * itsNpolarisations) + el2 * itsNpolarisations;
	  int offsetIm = el2 * (itsNelements * itsNpolarisations) + el1 * itsNpolarisations;
	  for (int pol=0; pol<itsNpolarisations; pol++){
	    freqBlock[ offsetIm + pol ] = dhp->getBufferElement(el1, el2, channel, pol)->imag();
	    freqBlock[ offsetRe + pol ] = dhp->getBufferElement(el1, el2, channel, pol)->real();
	  }
	}
      }
      written = write(itsOutputFile, freqBlock, itsNelements * itsNelements * itsNpolarisations * sizeof(DH_Vis::BufferPrimitive));
#define DUMP_NOT_DEFINED
#ifdef DUMP
      for (int el1=0; el1<itsNelements; el1++){
	cout<<el1<<": ";
	for (int el2=0; el2<itsNelements; el2++){
	  int offset = el1 * (itsNelements * itsNpolarisations) + el2 * itsNpolarisations;
	  for (int pol=0; pol<itsNpolarisations; pol++){
	    cout<<freqBlock[offset+pol]<<" ";
	  }
	  cout<<"  ";
	}
	cout<<endl;
      }
#endif
      
      if (written == -1) {
	cerr<<"Something went wrong during write!"<<endl;
      }
      totalWritten += written;
    }
  }
  recSize = recSize / 1024; // from now recSize is in kB
  totalWrittenKB = totalWritten / 1024;
  gettimeofday(&newTime, NULL);
  if (itsLastTime.tv_sec != 0) {
    double elapsed = newTime.tv_sec - itsLastTime.tv_sec + 10e-6*(newTime.tv_usec - itsLastTime.tv_usec);
    itsBandwidth = recSize/elapsed;
    cout << "Received " 
	 << recSize << " kB (" 
	 << itsBandwidth / elapsed << " kB/s), written " 
	 << totalWrittenKB << " kB (" 
	 << totalWrittenKB / elapsed <<" kB/s)"
	 << endl;    
  }

  memcpy(&itsLastTime, &newTime, sizeof(struct timeval));
}

void WH_Dump::postProcess() {
  close(itsOutputFile);
  itsOutputFile = 0;
}

void WH_Dump::dump() {
}
		 
