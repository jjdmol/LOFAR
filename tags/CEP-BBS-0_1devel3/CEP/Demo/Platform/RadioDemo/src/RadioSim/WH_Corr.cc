// WH_Corr.cpp: implementation of the WH_Corr class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "WH_Corr.h"
#include "callspectralfft.h"
#include "Profiler.h"
#include <fstream.h>

short WH_Corr::itsInstanceCnt = 0;
unsigned int WH_Corr::itsCurrentTimeStamp=0;
int WH_Corr::itsProcessProfilerState=0; 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



WH_Corr::WH_Corr (int inputs,
		  int outputs):
  WorkHolder (inputs, outputs),
  itsFile(NULL),
  itsXOffset(0)
{
  int ch;
  itsInDataHolders.reserve(inputs);
  itsOutDataHolders.reserve(outputs);
  for (ch = 0; ch < inputs; ch++)   {
    DH_BeamBand* aDH = new DH_BeamBand();
    itsInDataHolders.push_back(aDH);
  }
  for (ch = 0; ch < outputs; ch++)  {
    DH_Corr* aDH = new DH_Corr();
    itsOutDataHolders.push_back(aDH);
  }
  myInstanceCnt = itsInstanceCnt++;

  if (itsProcessProfilerState == 0) {
    itsProcessProfilerState = Profiler::defineState("Correlator_Process","grey");
  }
}


WH_Corr::~WH_Corr ()
{
}

void WH_Corr::process () {

  if (WorkHolder::getProcMode() == Process) {
    int input, freq;		// loop counters
    float realpart, imagpart;
    TRACER(debug,"WH_Corr  Process");
    Profiler::enterState (itsProcessProfilerState);
	
    
    // test the synchronisation of the time stamps.
    for (input = 0; input < getInputs (); input++) {
      if ( itsInDataHolders[input]->getPacket()->timeStamp !=
	   itsInDataHolders[0]->getPacket()->timeStamp) {
	Firewall::hit( __HERE__,
		       "OUT OF SYNC on input %i (%i <> %i)",
		       input,
		       itsInDataHolders[input]->getPacket()->timeStamp,
		       itsInDataHolders[0]->getPacket()->timeStamp);	  
      }
    }
    
    // keep track of the current timestamp for all correlators
    if (itsCurrentTimeStamp !=  itsInDataHolders[0]->getPacket()->timeStamp) {
      if ( ((itsCurrentTimeStamp+1) != itsInDataHolders[0]->getPacket()->timeStamp) 
	   && (itsCurrentTimeStamp != 0)){ // skip test for first timestamp
	Firewall::hit(__HERE__,
		      "Correlator: TimeStamps not consecutive %i %i",
		      itsCurrentTimeStamp, 
		      itsInDataHolders[0]->getPacket()->timeStamp);
      }
      itsCurrentTimeStamp = itsInDataHolders[0]->getPacket()->timeStamp;
//          cout << "Correlator processing timestamp : " 
//    	   << itsInDataHolders[0]->getPacket()->timeStamp << endl;
    }
   
    TRACER(debug,"Start loops");
    for (int station1 = 0; station1 < STATIONS; station1++) {
      for (int station2 = 0; station2 < station1; station2++) {
	for (freq = 0; freq < CORRFREQS; freq++)
	  {
	    realpart =
	      itsInDataHolders[station1]->getBuffer()[freq].real () *
	      itsInDataHolders[station2]->getBuffer()[freq].real () -
	      itsInDataHolders[station1]->getBuffer()[freq].imag () *
	      itsInDataHolders[station2]->getBuffer()[freq].imag ();
	    imagpart =
	      itsInDataHolders[station1]->getBuffer()[freq].imag () *
	      itsInDataHolders[station2]->getBuffer()[freq].real () +
	      itsInDataHolders[station1]->getBuffer()[freq].real () *
	      itsInDataHolders[station2]->getBuffer()[freq].imag ();
	    
	   
	    itsOutDataHolders[0]->getBuffer()[station1][station2][freq] = 
	      DataBufferType (realpart, imagpart);
	  }
	float corrcoef;
	pearsn(&itsInDataHolders[station1]->getBuffer()[0],
	       &itsInDataHolders[station2]->getBuffer()[0],
	       ANTSAMPLES-1,
	       &corrcoef);
//    	cout << "Corrcoeff " << station1 << " - " << station2  
//    	     << " = " << corrcoef
//    	     << endl;
		writeFile(corrcoef);
      }
    }
  } else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      break;
    case Ones :
      break;
    case Infile :
      {
	// read some infile ..
      }     
      break;
    case Skip :
      TRACER(error, "WH_Corr Error: cannot skip proces");
      break; 
    default :
      TRACER(error, "WH_Corr Error: unknown process mode ");
      break;
    }
  }
  Profiler::leaveState (itsProcessProfilerState);
  TRACER(debug,"Quit Corr");
}

void WH_Corr::openFile(char* filename)
{
  //cout << "open file " << filename << " on Correlator " << getName() << endl;
  itsFile = fopen(filename, "a");

  Firewall::Assert(NULL != itsFile, __HERE__, "WH_Corr::openFile");

  //itsXOffset = (-LAGS) / 2;
}

void WH_Corr::closeFile(void)
{
  //cout << "close file  on Correlator " << getName() << endl;
  fclose(itsFile);
}

void WH_Corr::writeFile(float y)
{
  Firewall::Assert(itsFile != NULL,
		   __HERE__,
		   "file pointer == NULL");
  fprintf(itsFile, "%d, %f\n", ++itsXOffset, y);
  printf("%d, %f\n", ++itsXOffset, y);
}

void WH_Corr::dump () const
{
//   cout << "WH_Corr Buffer_" << endl;
//    for (int station1 = 0; station1 < getInputs (); station1++) {
//       for (int station2 = 0; station2 < station1; station2++) {
// 	  cout << "Station" << station1 << " X " << station2 << " : ";
// 	  for (int freq = 0; freq < CORRFREQS; freq++) {
// 	      cout << itsOutDataHolders[0]->getBuffer()[station1][station2][freq];
// 	    }
// 	  cout << endl;
// 	}
//     }
//   cout << endl;
  // the stations to be printed (station1 > station 2 !!)
  if (getInstanceCnt() == 0) { // first MakeMS on dataprocessor
    ofstream outfile("outData/Corr.ext");
    cout << "Write Corr output frequency spectrum for only one correlation to file: outData/Corr.ext" << endl
	 << "For plotting in Matlab with: plot(load('outData/Corr.ext'))" << endl ;
    int station1 = STATIONS-1;
    int station2 = 0;
    for (int freq = 0; freq < CORRFREQS; freq++) {
      outfile << sqrt(itsOutDataHolders[0]->getBuffer()[station1][station2][freq].real () *
		      itsOutDataHolders[0]->getBuffer()[station1][station2][freq].real () +
		      itsOutDataHolders[0]->getBuffer()[station1][station2][freq].imag () *
		      itsOutDataHolders[0]->getBuffer()[station1][station2][freq].imag ())
	      << " ";
    }
    cout << endl;
  }
}


#include <math.h>
#define TINY 1.0e-20

void WH_Corr::pearsn(DataBufferType x[], 
		     DataBufferType y[], 
		     unsigned long n, 
		     float *r)
{
  //float betai(float a, float b, float x);
  //float erfcc(float x);
	unsigned long j;
	float yt,xt,t,df;
	float syy=0.0,sxy=0.0,sxx=0.0,ay=0.0,ax=0.0;

	for (j=1;j<=n;j++) {
		ax += abs(x[j]);
		ay += abs(y[j]);
	}
	ax /= n;
	ay /= n;
	for (j=1;j<=n;j++) {
		xt=abs(x[j])-ax;
		yt=abs(y[j])-ay;
		sxx += xt*xt;
		syy += yt*yt;
		sxy += xt*yt;
	}
	*r=sxy/(sqrt(sxx*syy)+TINY);
	//*z=0.5*log((1.0+(*r)+TINY)/(1.0-(*r)+TINY));
	df=n-2;
	t=(*r)*sqrt(df/((1.0-(*r)+TINY)*(1.0+(*r)+TINY)));
	//*prob=betai(0.5*df,0.5,df/(df+t*t));
}
#undef TINY
