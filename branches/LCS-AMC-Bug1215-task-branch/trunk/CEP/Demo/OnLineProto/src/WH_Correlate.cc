//  WH_Correlate.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>             // for sprintf
#include <Common/Debug.h>

// CEPFrame general includes
#include "CEPFrame/DH_Empty.h"
#include "CEPFrame/Step.h"
#include <Blob/KeyValueMap.h>

// OnLineProto specific include
#include "OnLineProto/WH_Correlate.h"

namespace LOFAR
{

WH_Correlate::WH_Correlate (const string& name,
			    unsigned int channels,
			    const ACC::APS::ParameterSet& ps)
  : WorkHolder    (channels, channels, name,"WH_Correlate"),
    itsNelements  (ps.getInt32("general.nstations")),
    itsNitems     (ps.getInt32("corr.tsize")),
    itsPS         (ps)
{
  char str[8];
  // create the input dataholders
  for (unsigned int i=0; i<channels; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_CorrCube (string("Corr in_") + str, ps));
  }
  // create the output dataholders
  for (unsigned int i=0; i<channels; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_Vis (string("Corr out_") + str, ps));
  }
}

WH_Correlate::~WH_Correlate()
{
}

WorkHolder* WH_Correlate::construct (const string& name, 
				     unsigned int channels,
				     const ACC::APS::ParameterSet& ps)
{
  return new WH_Correlate (name, channels, ps);
}

WH_Correlate* WH_Correlate::make (const string& name)
{
  return new WH_Correlate (name, 
			   getDataManager().getInputs(),
			   itsPS);
}

void WH_Correlate::process()
{
  
  TRACER4("WH_Correlate::Process()");

  DH_CorrCube* InDHptr;
  DH_Vis*      OutDHptr;

  // ToDo: set all output counters to zero

  // Select the third frequency band, since this contains our signal
  InDHptr = (DH_CorrCube*)getDataManager().getInHolder(0);
  OutDHptr = (DH_Vis*)getDataManager().getOutHolder(0);

  blitz::Array<complex<float>, 2> signal (itsNelements, itsNitems);
  blitz::Array<complex<float>, 2> corr (itsNelements, itsNelements);

//   for (int i = 0; i < itsNelements; i++) {
//     for (int j = 0; j < itsNitems; j++) {
      
//       if (i == 0) signal (i,j) = complex<float> (0.05, 0.85);
//       else signal (i,j) = complex<float> (-0.2, 0.03);
      
//     }
//   }

  // Enter data either by assignment(1) or by memcpy(2) 
  // or by DH_CorrCube accessor(3); in order of expected performance

  
  //1 signal = *InDHptr->getBuffer(); // DOESN't work
  memcpy(signal.data(), InDHptr->getBuffer(), itsNelements*itsNitems*sizeof(DH_CorrCube::BufferType));

//    for (int element = 0; element < itsNelements; element++) {
//      for (int item = 0; item < itsNitems; item++) {
//        signal(element,item) = *(InDHptr->getBufferElement(element,item,0));
//      }
//    }
//    cout << signal(5,5) << " ";
//    cout << endl;


  corr = complex<float> (0,0);

  //  WH_Correlate::correlator_core(signal, corr);
  WH_Correlate::correlator_core_unrolled(signal, corr);
  
  // copy the correlation matrix to the output
  memcpy(OutDHptr->getBuffer(), corr.data(), itsNelements*itsNelements*sizeof(DH_Vis::BufferType));
}


void WH_Correlate::dump()
{
}


void WH_Correlate::correlator_core(blitz::Array<complex<float>, 2>& signal, 
				   blitz::Array<complex<float>, 2>& corr) {

  int x, y;
  for (int time = 0; time < itsNitems; time++) {
    for (x = 0; x < itsNelements; x++) {
      for (y = 0; y <= x; y++) {
	corr(x,y) += complex<float> (
		      signal(x, time).real() * signal(y, time).real() -  // real 
		      signal(x, time).imag() * signal(y, time).imag(), 
		      
		      signal(x, time).real() * signal(y, time).imag() +  // imag
		      signal(x, time).imag() * signal(y, time).real()
		      );
      }
    }
  }
}

void WH_Correlate::correlator_core_unrolled(blitz::Array<complex<float>, 2>& s,
					    blitz::Array<complex<float>, 2>& c) {

  int loop = 5;
  int x, y;

  for ( int time = 0; time < itsNitems; time++) {
  for ( x = 0; (x+loop) < itsNelements; x += loop ) {
    for ( y = 0; (y+loop) <= x; y += loop ) {

      c(x  ,y  ) += complex<float> (
		     s(x  ,time).real() * s(y  ,time).real() - s(x  ,time).imag() * s(y  ,time).imag(),
		     s(x  ,time).real() * s(y  ,time).imag() + s(x  ,time).imag() * s(y  ,time).real()
		     );
      
      c(x  ,y+1) += complex<float> (
		     s(x  ,time).real() * s(y+1,time).real() - s(x  ,time).imag() * s(y+1,time).imag(),
		     s(x  ,time).real() * s(y+1,time).imag() + s(x  ,time).imag() * s(y+1,time).real()
		     );
      
      c(x  ,y+2) += complex<float> (
		     s(x  ,time).real() * s(y+2,time).real() - s(x  ,time).imag() * s(y+2,time).imag(),
		     s(x  ,time).real() * s(y+2,time).imag() + s(x  ,time).imag() * s(y+2,time).real()
		     );
      
      c(x  ,y+3) += complex<float> (
		     s(x  ,time).real() * s(y+3,time).real() - s(x  ,time).imag() * s(y+3,time).imag(),
		     s(x  ,time).real() * s(y+3,time).imag() + s(x  ,time).imag() * s(y+3,time).real()
		     );
      
      c(x  ,y+4) += complex<float> (
		     s(x  ,time).real() * s(y+4,time).real() - s(x  ,time).imag() * s(y+4,time).imag(),
		     s(x  ,time).real() * s(y+4,time).imag() + s(x  ,time).imag() * s(y+4,time).real() 
		     );
      
      
      
      c(x+1,y  ) += complex<float> (
		     s(x+1,time).real() * s(y  ,time).real() - s(x+1,time).imag() * s(y  ,time).imag(),
		     s(x+1,time).real() * s(y  ,time).imag() + s(x+1,time).imag() * s(y  ,time).real()
		     );
      
      c(x+1,y+1) += complex<float> (
		     s(x+1,time).real() * s(y+1,time).real() - s(x+1,time).imag() * s(y+1,time).imag(),
		     s(x+1,time).real() * s(y+1,time).imag() + s(x+1,time).imag() * s(y+1,time).real()
		     );
      
      c(x+1,y+2) += complex<float> (
		     s(x+1,time).real() * s(y+2,time).real() - s(x+1,time).imag() * s(y+2,time).imag(),
		     s(x+1,time).real() * s(y+2,time).imag() + s(x+1,time).imag() * s(y+2,time).real()
		     );
      
      c(x+1,y+3) += complex<float> (
		     s(x+1,time).real() * s(y+3,time).real() - s(x+1,time).imag() * s(y+3,time).imag(),
		     s(x+1,time).real() * s(y+3,time).imag() + s(x+1,time).imag() * s(y+3,time).real() 
		     );
      
      c(x+1,y+4) += complex<float> (
		     s(x+1,time).real() * s(y+4,time).real() - s(x+1,time).imag() * s(y+4,time).imag(),
		     s(x+1,time).real() * s(y+4,time).imag() + s(x+1,time).imag() * s(y+4,time).real()
		     );
      
      
      c(x+2,y  ) += complex<float> (
		     s(x+2,time).real() * s(y  ,time).real() - s(x+2,time).imag() * s(y  ,time).imag(),
		     s(x+2,time).real() * s(y  ,time).imag() + s(x+2,time).imag() * s(y  ,time).real()
		     );
      
      c(x+2,y+1) += complex<float> (
		     s(x+2,time).real() * s(y+1,time).real() - s(x+2,time).imag() * s(y+1,time).imag(),
		     s(x+2,time).real() * s(y+1,time).imag() + s(x+2,time).imag() * s(y+1,time).real()
		     );
      
      c(x+2,y+2) += complex<float> (
		     s(x+2,time).real() * s(y+2,time).real() - s(x+2,time).imag() * s(y+2,time).imag(),
		     s(x+2,time).real() * s(y+2,time).imag() + s(x+2,time).imag() * s(y+2,time).real()
		     );
      
      c(x+2,y+3) += complex<float> (
		     s(x+2,time).real() * s(y+3,time).real() - s(x+2,time).imag() * s(y+3,time).imag(),
		     s(x+2,time).real() * s(y+3,time).imag() + s(x+2,time).imag() * s(y+3,time).real()
		     );
      
      c(x+2,y+4) += complex<float> (
		     s(x+2,time).real() * s(y+4,time).real() - s(x+2,time).imag() * s(y+4,time).imag(),
		     s(x+2,time).real() * s(y+4,time).imag() + s(x+2,time).imag() * s(y+4,time).real()
		     );
      
      
      c(x+3,y  ) += complex<float> (
		     s(x+3,time).real() * s(y  ,time).real() - s(x+3,time).imag() * s(y  ,time).imag(),
		     s(x+3,time).real() * s(y  ,time).imag() + s(x+3,time).imag() * s(y  ,time).real()
		     );
      
      c(x+3,y+1) += complex<float> (
		     s(x+3,time).real() * s(y+1,time).real() - s(x+3,time).imag() * s(y+1,time).imag(),
		     s(x+3,time).real() * s(y+1,time).imag() + s(x+3,time).imag() * s(y+1,time).real()
		     );
      
      c(x+3,y+2) += complex<float> (
		     s(x+3,time).real() * s(y+2,time).real() - s(x+3,time).imag() * s(y+2,time).imag(),
		     s(x+3,time).real() * s(y+2,time).imag() + s(x+3,time).imag() * s(y+2,time).real()
		     );
      
      c(x+3,y+3) += complex<float> (
		     s(x+3,time).real() * s(y+3,time).real() - s(x+3,time).imag() * s(y+3,time).imag(),
		     s(x+3,time).real() * s(y+3,time).imag() + s(x+3,time).imag() * s(y+3,time).real()
		     );
      
      c(x+3,y+4) += complex<float> (
		     s(x+3,time).real() * s(y+4,time).real() - s(x+3,time).imag() * s(y+4,time).imag(),
		     s(x+3,time).real() * s(y+4,time).imag() + s(x+3,time).imag() * s(y+4,time).real()
		     );
      
      
      c(x+4,y  ) += complex<float> (
		     s(x+4,time).real() * s(y  ,time).real() - s(x+4,time).imag() * s(y  ,time).imag(),
		     s(x+4,time).real() * s(y  ,time).imag() + s(x+4,time).imag() * s(y  ,time).real()
		     );
      
      c(x+4,y+1) += complex<float> (
		     s(x+4,time).real() * s(y+1,time).real() - s(x+4,time).imag() * s(y+1,time).imag(),
		     s(x+4,time).real() * s(y+1,time).imag() + s(x+4,time).imag() * s(y+1,time).real()
		     );
      
      c(x+4,y+2) += complex<float> (
		     s(x+4,time).real() * s(y+2,time).real() - s(x+4,time).imag() * s(y+2,time).imag(),
		     s(x+4,time).real() * s(y+2,time).imag() + s(x+4,time).imag() * s(y+2,time).real()
		     );
      
      c(x+4,y+3) += complex<float> (
		     s(x+4,time).real() * s(y+3,time).real() - s(x+4,time).imag() * s(y+3,time).imag(),
		     s(x+4,time).real() * s(y+3,time).imag() + s(x+4,time).imag() * s(y+3,time).real()
		     );
      
      c(x+4,y+4) += complex<float> (
		     s(x+4,time).real() * s(y+4,time).real() - s(x+4,time).imag() * s(y+4,time).imag(),
		     s(x+4,time).real() * s(y+4,time).imag() + s(x+4,time).imag() * s(y+4,time).real()
		     );
      
    }
    /* Process the leftovers */
    c(x  ,y  ) += complex<float> (
		   s(x  ,time).real() * s(y  ,time).real() - s(x  ,time).imag() * s(y  ,time).imag(),
		   s(x  ,time).real() * s(y  ,time).imag() + s(x  ,time).imag() * s(y  ,time).real()
		   );
    
    c(x+1,y  ) += complex<float> (
		   s(x+1,time).real() * s(y  ,time).real() - s(x+1,time).imag() * s(y  ,time).imag(),
		   s(x+1,time).real() * s(y  ,time).imag() + s(x+1,time).imag() * s(y  ,time).real()
		   );
    
    c(x+1,y+1) += complex<float> (
		   s(x+1,time).real() * s(y+1,time).real() - s(x+1,time).imag() * s(y+1,time).imag(),
		   s(x+1,time).real() * s(y+1,time).imag() + s(x+1,time).imag() * s(y+1,time).real()
		   );
    
    c(x+2,y  ) += complex<float> (
		   s(x+2,time).real() * s(y  ,time).real() - s(x+2,time).imag() * s(y  ,time).imag(),
		   s(x+2,time).real() * s(y  ,time).imag() + s(x+2,time).imag() * s(y  ,time).real()
		   );
    
    c(x+2,y+1) += complex<float> (
		   s(x+2,time).real() * s(y+1,time).real() - s(x+2,time).imag() * s(y+1,time).imag(),
		   s(x+2,time).real() * s(y+1,time).imag() + s(x+2,time).imag() * s(y+1,time).real()
		   );
    
    c(x+2,y+2) += complex<float> (
		   s(x+2,time).real() * s(y+2,time).real() - s(x+2,time).imag() * s(y+2,time).imag(),
		   s(x+2,time).real() * s(y+2,time).imag() + s(x+2,time).imag() * s(y+2,time).real()
		   );
    
    
    c(x+3,y  ) += complex<float> (
		   s(x+3,time).real() * s(y  ,time).real() - s(x+3,time).imag() * s(y  ,time).imag(),
		   s(x+3,time).real() * s(y  ,time).imag() + s(x+3,time).imag() * s(y  ,time).real()
		   );
    
    c(x+3,y+1) += complex<float> (
		   s(x+3,time).real() * s(y+1,time).real() - s(x+3,time).imag() * s(y+1,time).imag(),
		   s(x+3,time).real() * s(y+1,time).imag() + s(x+3,time).imag() * s(y+1,time).real()
		   );
    
    c(x+3,y+2) += complex<float> (
		   s(x+3,time).real() * s(y+2,time).real() - s(x+3,time).imag() * s(y+2,time).imag(),
		   s(x+3,time).real() * s(y+2,time).imag() + s(x+3,time).imag() * s(y+2,time).real()
		   );
    
    c(x+3,y+3) += complex<float> (
		   s(x+3,time).real() * s(y+3,time).real() - s(x+3,time).imag() * s(y+3,time).imag(),
		   s(x+3,time).real() * s(y+3,time).imag() + s(x+3,time).imag() * s(y+3,time).real()
		   );
    
    c(x+4,y  ) += complex<float> (
		   s(x+4,time).real() * s(y  ,time).real() - s(x+4,time).imag() * s(y  ,time).imag(),
		   s(x+4,time).real() * s(y  ,time).imag() + s(x+4,time).imag() * s(y  ,time).real()
		   );
    
    c(x+4,y+1) += complex<float> (
		   s(x+4,time).real() * s(y+1,time).real() - s(x+4,time).imag() * s(y+1,time).imag(),
		   s(x+4,time).real() * s(y+1,time).imag() + s(x+4,time).imag() * s(y+1,time).real()
		   );
    
    c(x+4,y+2) += complex<float> (
		   s(x+4,time).real() * s(y+2,time).real() - s(x+4,time).imag() * s(y+2,time).imag(),
		   s(x+4,time).real() * s(y+2,time).imag() + s(x+4,time).imag() * s(y+2,time).real()
		   );
    
    c(x+4,y+3) += complex<float> (
		   s(x+4,time).real() * s(y+3,time).real() - s(x+4,time).imag() * s(y+3,time).imag(),
		   s(x+4,time).real() * s(y+3,time).imag() + s(x+4,time).imag() * s(y+3,time).real()
		   );
    
    c(x+4,y+4) += complex<float> (
		   s(x+4,time).real() * s(y+4,time).real() - s(x+4,time).imag() * s(y+4,time).imag(),
		   s(x+4,time).real() * s(y+4,time).imag() + s(x+4,time).imag() * s(y+4,time).real()
		   );
    
  }
  }
}
}// namespace LOFAR
