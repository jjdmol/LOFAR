// WH_MakeMS.cpp: implementation of the WH_MakeMS class.
//
//////////////////////////////////////////////////////////////////////

#include <fstream.h>
#include "WH_MakeMS.h"

short WH_MakeMS::theirInstanceCnt = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


WH_MakeMS::WH_MakeMS (int ninputs, LSFiller* filler)
: WorkHolder (ninputs, 1),
  itsFiller  (filler),
  itsCounter (0)
{
  itsInDataHolders.reserve(ninputs);
  for (int ch=0; ch<ninputs; ch++)    {
    DH_Corr* aDH = new DH_Corr();
    itsInDataHolders.push_back(aDH);
  }
  itsInstanceCnt = theirInstanceCnt++;
}


WH_MakeMS::~WH_MakeMS()
{
  for (int ch=0; ch<getInputs(); ch++) {
    delete itsInDataHolders[ch];
  }
}

void WH_MakeMS::process()
{
  // read the input dataholders
  TRACER(debug,"WH_MakeMS  Process");
  // start processing; actual processing mode depends on ProcMode.
  if (WorkHolder::getProcMode() == Process) {
    // Write all data into the MS.
    // The order is beam,band,freq.
    for (int input=0; input < getInputs(); input++) {
      int nrdata = sizeof(DH_Corr::Corr_BType) / sizeof(DataBufferType);
      int bandnr = input%FCORR;
      int fieldnr = input/FCORR;
      itsFiller->write (bandnr, fieldnr, itsCounter, nrdata,
      		     (complex<float>*)(itsInDataHolders[input]->getBuffer()));
    }
    itsCounter++;
  } else { // zeroes,ones,infile,skip
    switch (WorkHolder::getProcMode()) {
    case Zeroes :
      cout << "WH_MakaMS::Process-zeroes not implemented" << endl;
    break;
    case Ones :
      cout << "WH_MakaMS::Process-ones not implemented" << endl;
    break;
    case Infile :
      cout << "WH_MakaMS::Process-infile not implemented" << endl;
      break;
    case Skip :
      cout << "WH_MakaMS::Process-skip not implemented" << endl;
      break;
    default :
      TRACER(error, "WH_MakeMS Error: unknown process mode ");
      break;
    }
  }
}

void WH_MakeMS::dump() const
{
   cout << "WH_MakeMS INPUT Buffer_" << endl;
   for (int station1 = 0; station1 < STATIONS; station1++) {
     for (int station2 = 0; station2 < station1; station2++) {
       for (int freq = 0; freq < CORRFREQS; freq++) {
 	cout << itsInDataHolders[0]->getBuffer()[station1][station2][freq];
       }
       cout << endl;
       }
   }
   cout << endl;
  
  // the stations to be printed (station1 > station 2 !!)
  if (getInstanceCnt() == 0) { // first correlator on dataprocessor
    ofstream outfile("MakeMS.ext");
    cout << "Write MakeMS input frequency spectrum for only one correlation to file: outData/Corr.ext" << endl
	 << "For plotting in Matlab with: plot(load('outData/Corr.ext'))" << endl ;

    int station1 = STATIONS-1;
    int station2 = 0;
    for (int freq = 0; freq < CORRFREQS; freq++) {
      outfile << sqrt(itsInDataHolders[0]->getBuffer()[station1][station2][freq].real () *
		      itsInDataHolders[0]->getBuffer()[station1][station2][freq].real () +
		      itsInDataHolders[0]->getBuffer()[station1][station2][freq].imag () *
		      itsInDataHolders[0]->getBuffer()[station1][station2][freq].imag ())
	      << " ";
    }
    cout << endl;
  }
}
