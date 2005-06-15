//#  tWH_Transpose.cc:
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

#include <Common/Exception.h>
#include <Common/lofar_iostream.h>
#include <TFC_Interface/RectMatrix.h>

#include <iostream>

using namespace LOFAR;

int main (int argc, const char** argv){

  try {
    // define the matrix dimensions
    vector<DimDef> vdd;
    vdd.push_back(DimDef("tijd", 2));
    vdd.push_back(DimDef("freq", 3));
    vdd.push_back(DimDef("chan", 4));
    RectMatrix<int> myMatrix(vdd);

    // allocate the memory for the matrix
    int memsize = 2*3*4*sizeof(int);
    char buffer[memsize];
    memset(buffer, 0, memsize);
    myMatrix.setBuffer((int*)buffer, 2*3*4);
    
    // get the dimension ids from the matrix
    dimType tijdDim, freqDim, chanDim;
    tijdDim = myMatrix.getDim("tijd");
    freqDim = myMatrix.getDim("freq");
    chanDim = myMatrix.getDim("chan");

    // test 1: set matrix elements
    int value = 0;
    RectMatrix<int>::cursorType beginCursor = myMatrix.getCursor(0*tijdDim + 0*freqDim + 0*chanDim);
    RectMatrix<int>::cursorType tcursor, fcursor, ccursor;
asm volatile ("nop");
    for (int t=0, tcursor = beginCursor; t<myMatrix.getNElemInDim(tijdDim); myMatrix.moveCursor(&tcursor, tijdDim), t++) {
      for (int f=0, fcursor = tcursor; f<myMatrix.getNElemInDim(freqDim); myMatrix.moveCursor(&fcursor, freqDim), f++) {
	for (int c=0, ccursor = fcursor; c<myMatrix.getNElemInDim(chanDim); myMatrix.moveCursor(&ccursor, chanDim), c++) {
	  //	  cerr<<"setting value : "<<value<<endl;
	  myMatrix.setValue(ccursor, value++);
	}
      }
    }
asm volatile ("nop");

    // now readout all values
    value=0;
    tcursor = myMatrix.getCursor(0*tijdDim + 0*freqDim + 0*chanDim);
    MATRIX_FOR_LOOP(myMatrix, tijdDim, tcursor) {
      fcursor = tcursor;
      MATRIX_FOR_LOOP(myMatrix, freqDim, fcursor) {
	ccursor = fcursor;
	MATRIX_FOR_LOOP(myMatrix, chanDim, ccursor) {
	  //	  cerr<<myMatrix.getValue(ccursor)<<" ?= "<<value<<endl;
	  myMatrix.setValue(ccursor, value++);
	  //	  ASSERTSTR(myMatrix.getValue(ccursor) == value++, "wrong value found in matrix");
	}
      }
    };
    asm volatile ("nop");
    cerr<<value<<endl;
#if 0
    // the above paragraph with all macros and inlines expanded
    // only the map lookups are slow, we should do something about that

    // the map lookups can be replaces. But then we need to redefine dim
    // all operations would cost a lookup in an array
    // but if the users types the for-loops himself, he can do the map lookup once
    // and remember the result
    
    // the cursors are pointers to the valueType (which is int in this program)
    // the dims are ints
    value = 0;
    tcursor = myMatrix.itsData + (0*tijdDim + 0*freqDim + 0*chanDim);
    { 
      int cursorMAX = myMatrix.itsDimSizeMap[tijdDim];
      for (int cursorIT = 0; cursorIT < cursorMAX; cursorIT ++ ) {
	fcursor = tcursor;
	{ 
	  int cursorMAX = myMatrix.itsDimSizeMap[dim]; 
	  for (int cursorIT = 0; cursorIT < cursorMAX; cursorIT ++ ) {
	    ccursor = fcursor;
	    { 
	      int cursorMAX = myMatrix.itsDimSizeMap[dim]; 
	      for (int cursorIT = 0; cursorIT < cursorMAX; cursorIT ++ ) {
		cerr<<*ccursor<<" ?= "<<value<<endl;
		ASSERTSTR(*ccursor == value++, "wrong value found in matrix");
		ccursor += chanDim;
	      } 
	    }   
	    fcursor += freqDim;
	  } 
	}    
	tcursor += tijdDim;
      } 
    }
#endif

    // TODO test 2 : copy matrix block
    int srcBuffer[12];
    memset (srcBuffer, 0, 12*sizeof(int));
    RectMatrix<int>::cursorType cpyCursor = myMatrix.getCursor(1*tijdDim);
    myMatrix.cpyFromBlock((int*)srcBuffer, 12, cpyCursor, tijdDim, 1);
    {
    value = 0;
    tcursor = myMatrix.getCursor(0*tijdDim + 0*freqDim + 0*chanDim);
    MATRIX_FOR_LOOP(myMatrix, tijdDim, tcursor) {
      fcursor = tcursor;
      MATRIX_FOR_LOOP(myMatrix, freqDim, fcursor) {
	ccursor = fcursor;
	MATRIX_FOR_LOOP(myMatrix, chanDim, ccursor) {
	  //cerr<<myMatrix.getValue(ccursor)<<endl;
	  if (value > 11) {
	    ASSERTSTR(myMatrix.getValue(ccursor) == 0, "wrong value in matrix after cpyBlock");
	  } else {
	    ASSERTSTR(myMatrix.getValue(ccursor) == value++, "wrong value found in matrix");
	  }
	}
      }
    }
    }
    // TODO also add tests to see if copying imcompatible blocks goes wrong


  } catch (LOFAR::Exception e) {
    cerr << "Caught exception: "<< e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Caught exception "<< endl;
    exit(1);
  }
  return 0;
}
