//#  tDH_FIR.cc: Test DH_FIR
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <Common/lofar_complex.h>
#include <Transport/Connection.h>
#include <Transport/TH_Mem.h>
#include <DH_FIR.h>

using namespace LOFAR;

int main (int argc, char **argv) {

  try {
    DH_FIR srcDH("subband", 1);
    DH_FIR dstDH("subband", 1);
    srcDH.init();
    dstDH.init();
    TH_Mem TH;
    Connection con("connection", &srcDH, &dstDH, &TH, false);    
    
    
    { // this is specific for DH_FIR
      RectMatrix<DH_FIR::BufferType>& srcMatrix = srcDH.getDataMatrix();
      dimType stationDim = srcMatrix.getDim("Station"); 
      dimType freqDim = srcMatrix.getDim("FreqChannel"); 
      dimType timeDim = srcMatrix.getDim("Time"); 
      dimType polDim = srcMatrix.getDim("Polarisation"); 

      // put some data in the DataHolder
      DH_FIR::BufferType value = makei16complex(0,0);
      RectMatrix<DH_FIR::BufferType>::cursorType beginCursor = srcMatrix.getCursor(0*stationDim + 0*freqDim + 0*polDim + 0*timeDim);
      RectMatrix<DH_FIR::BufferType>::cursorType scursor, fcursor, tcursor, pcursor;
      for (int s=0, scursor = beginCursor; s<srcMatrix.getNElemInDim(stationDim); srcMatrix.moveCursor(&scursor, stationDim), s++) {
	for (int f=0, fcursor = scursor; f<srcMatrix.getNElemInDim(freqDim); srcMatrix.moveCursor(&fcursor, freqDim), f++) {
	  for (int t=0, tcursor = fcursor; t<srcMatrix.getNElemInDim(timeDim); srcMatrix.moveCursor(&tcursor, timeDim), t++) {
	    for (int p=0, pcursor = tcursor; p<srcMatrix.getNElemInDim(polDim); srcMatrix.moveCursor(&pcursor, polDim), p++) {
//  	      srcMatrix.setValue(pcursor, value++);
	    }
	  }
	}
      }
    } // this is specific for DH_FIR

    // move data to dstDH using TH_Mem
    con.write();
    con.read();
    
    { // this is specific for DH_FIR
      // ask the dimensions in a different order
      RectMatrix<DH_FIR::BufferType>& dstMatrix = srcDH.getDataMatrix();
      dimType polDim = dstMatrix.getDim("Polarisation"); 
      dimType freqDim = dstMatrix.getDim("FreqChannel"); 
      dimType stationDim = dstMatrix.getDim("Station"); 
      dimType timeDim = dstMatrix.getDim("Time"); 

      // check the data in the dataHolder
      DH_FIR::BufferType value = makei16complex(0,0);
      RectMatrix<DH_FIR::BufferType>::cursorType scursor, fcursor, tcursor, pcursor;

      scursor = dstMatrix.getCursor(0*stationDim + 0*freqDim + 0*polDim + 0*timeDim);
      cerr<<"freqDim.total: "<<freqDim.total<<endl;
      MATRIX_FOR_LOOP(dstMatrix, stationDim, scursor) {
	fcursor = scursor;
	MATRIX_FOR_LOOP(dstMatrix, freqDim, fcursor) {
	  tcursor = fcursor;
	  MATRIX_FOR_LOOP(dstMatrix, timeDim, tcursor) {
	    pcursor = tcursor;
	    MATRIX_FOR_LOOP(dstMatrix, polDim, pcursor) {
	      cerr<<"Expected: "<<value<<" found: "<< dstMatrix.getValue(pcursor)<<endl;
//  	      ASSERTSTR(dstMatrix.getValue(pcursor) == value++, "wrong value found in matrix");
	    }
	  }
	}
      }
    }  // this is specific for DH_FIR
    
  } catch (LOFAR::Exception e) {
    cerr << "Caught exception: "<< e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Caught exception "<< endl;
    exit(1);
  }
  return 0;
}
