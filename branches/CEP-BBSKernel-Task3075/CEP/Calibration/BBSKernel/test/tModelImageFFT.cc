//# tModelImageFFT.cc: 
//#
//#
//# Copyright (C) 2012
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
//# $Id: tModelImageFFT.cc 20029 2012-05-14 20:50:23Z duscha $

#include <lofar_config.h>
#include <iostream>
#include <Common/Exception.h>     // THROW macro for exceptions
#include <BBSKernel/ModelImageFFT.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>

using namespace std;
using namespace casa;

unsigned int getBaselines(const MeasurementSet &ms, double *baselines[3], 
                  int rowStart=-1, int rowEnd=-1);
unsigned int getBaselines(const MeasurementSet &ms, double *u, double *v, double *w, 
                          int rowStart=-1, int rowEnd=-1);                  
Vector<Double> getMSFrequencies(const MeasurementSet &ms);
void addModelColumn(MeasurementSet& ms, const String& dataManName, unsigned int nchan=1);
string createColumnName(const string &ModelFilename);
void writeData( MeasurementSet &ms, const string &name, 
                const DComplex *XX, const DComplex *XY,
                const DComplex *YX, const DComplex *YY, 
                unsigned int nbaselines, unsigned int nfreqs=1, bool useFlags=False);
void printBaselines(double **baselines, size_t nrows);
void printBaselines(double *u, double *v, double *w, size_t nrows);

int main(int argc, char **argv)
{
  unsigned int nwplanes=1;      // number of w-planes to use (default 1)
  string msName;
  string imageFilename=argv[1];
  if(argc==3)
    msName=argv[2];
  if(argc==4)
    nwplanes=atoi(argv[3]);

  LOFAR::BBS::ModelImageFft modelImage(imageFilename, nwplanes);  // nwplanes=1
  MeasurementSet ms(msName, Table::Update);    // open MS

  /*
  double **baselines=static_cast<double**>(malloc(ms.nrow()*sizeof(double)));
  for(unsigned int i=0; i<3; i++)
  {
    baselines[i]=static_cast<double*>(malloc(sizeof(double)*3));
  }  
  */
  // allocate memory for array to hold baselines 
  double *u=static_cast<double*>(malloc(ms.nrow()*sizeof(double)));
  double *v=static_cast<double*>(malloc(ms.nrow()*sizeof(double)));
  double *w=static_cast<double*>(malloc(ms.nrow()*sizeof(double)));

  Vector<Double> MSfreqs=getMSFrequencies(ms);  // get MS frequencies
  double *frequencies=static_cast<double*>(malloc(MSfreqs.size()*sizeof(double)));
  for(uInt i=0; i<MSfreqs.size(); i++)
  {
    frequencies[i]=MSfreqs[i];
  }
  //vector<double> frequencies;                   // STL vector for MS frequencies
  //MSfreqs.tovector(frequencies);

  unsigned int nfreqs=MSfreqs.size();          // number of frequencies in MS (for sim)
  //cout << "MSfreqs: " << MSfreqs << endl;       // DEBUG

//  unsigned int nbaselines=getBaselines(ms, baselines);  // get baselines from MS
//  printBaselines(baselines, nbaselines);      // DEBUG
  unsigned int nbaselines=getBaselines(ms, u, v, w);
  //printBaselines(u, v, w, nbaselines);          // DEBUG

  // Allocate memory for correlations XX, XY, YX, YY
  DComplex *XX=(DComplex*)malloc(nbaselines*nfreqs*sizeof(DComplex));
  DComplex *YY=(DComplex*)malloc(nbaselines*nfreqs*sizeof(DComplex));

  modelImage.degrid(u, v, w, nbaselines, nfreqs, frequencies, XX, NULL, NULL, YY);
  string columnname=createColumnName(imageFilename);
  addModelColumn(ms, columnname);
  ms.flush(); // needed to make added column flush to file
  writeData(ms, columnname, XX, NULL, NULL, YY, nbaselines, nfreqs); // Write correlations to MS column

  /*
  // Function to get degridded data into raw pointers
  void degrid(const double *u, const double *v, const double *w, 
              size_t timeslots, size_t nfreqs,
              const double *frequencies, 
              casa::DComplex *XX , casa::DComplex *XY, 
              casa::DComplex *XY , casa::DComplex *YY,
              double maxBaseline=200000);
  */

  ms.flush();
  ms.closeSubTables();

  cout << "Done" << endl;
  return 0;
}

// Get baselines from MS from rowStart till rowEnd
//
unsigned int getBaselines(const MeasurementSet &ms, double *baselines[3], 
                          int rowStart, int rowEnd)   // range not supported yet
{
  // Example: 711.309, -357.693, 432.535
  // TODO: Check that rowStart and rowEnd are within the limits
  uInt nrows=ms.nrow();
  IPosition shape = ROTableColumn(ms, MS::columnName(MS::UVW)).shapeColumn();
 
  cout << "getBaselines(): nrows: " << nrows << endl;

  // ROTableColumn UVW: Double Array of Size [ 1 3 ]
  ROArrayColumn<Double> uvwColumn(ms, "UVW");
  Array<Double> uvw=uvwColumn.getColumn();
  for(unsigned int i=0; i<nrows; i++)
  {
    baselines[i]=uvw[i].data();   // assign column to array pointer
  }
  return nrows;                   // return number of baselines
}

unsigned int getBaselines(const MeasurementSet &ms, double *u, double *v, double *w, 
                          int rowStart, int rowEnd)   // range not supported yet
{
  // Example: 711.309, -357.693, 432.535
  uInt nrows=ms.nrow();
  IPosition shape = ROTableColumn(ms, MS::columnName(MS::UVW)).shapeColumn();
  ROArrayColumn<Double> uvwColumn(ms, "UVW");  // Double Array of Size [ 1 3 ]

  Array<Double> uvw=uvwColumn.getColumn();
  for(unsigned int i=0; i<nrows; i++)
  {
    u[i]=uvw[i].data()[0];   // assign column to array pointer
    v[i]=uvw[i].data()[1];   // assign column to array pointer
    w[i]=uvw[i].data()[2];   // assign column to array pointer
  } 
  return nrows;                   // return number of baselines
}


Vector<Double> getMSFrequencies(const MeasurementSet &ms)
{
  // Table: SPECTRAL_WINDOW, Column: CHAN_FREQ
  // Read MSfrequency from MS/SPECTRAL_WINDOW table
  Table SpectralWindowTable(ms.keywordSet().asTable("SPECTRAL_WINDOW"));
  ROArrayColumn<Double> ChanFreqColumn(SpectralWindowTable, "CHAN_FREQ");

  Vector<Double> MSfrequencies=ChanFreqColumn(0);
  return MSfrequencies;
}

// Write data into an ArrayColumn
//
void writeData( MeasurementSet &ms, const string &colName, const DComplex *XX, 
                const DComplex *XY, const DComplex *YX, const DComplex *YY,
                unsigned int nbaselines, unsigned int nfreqs, bool useFlags)
{
  // Complex Array of size [ 4 nchannels ]
  ArrayColumn<Complex> modelCol(ms, colName);
  IPosition shape(2, 4, nfreqs);

  ScalarColumn<Bool> flagRow(ms, "FLAG_ROW");

  for(unsigned int row=0; row<nbaselines*nfreqs; row++)
  {
    Bool flag=flagRow(row);    
    if(!flag)
    {  
      modelCol.setShape(row, shape);    // set shape to 4 Corr, N frequencies
      Array<Complex> correlations(shape);
      correlations.set(0);              // initialize with 0
  
      // gather correlations into temporary array
      if(XX)
      {
        IPosition pos(2, 0, row%nfreqs);
        correlations(pos)=XX[row];
      }
      if(XY)
      {
        IPosition pos(2, 1, row%nfreqs);  
        correlations(pos)=XY[row];
      }
      if(YX)
      {
        IPosition pos(2, 2, row%nfreqs);  
        correlations(pos)=YX[row];
      }
      if(YY)
      {
        IPosition pos(2, 3, row%nfreqs);      
        correlations(pos)=YY[row];
      }    
      //cout << "writeData(): correlations: " << correlations << endl;   // DEBUG
      modelCol.put(row, correlations);  // 	Put the array in row's cell
    }
  }
}

void addModelColumn (MeasurementSet& ms, const String& dataManName, unsigned int nchan)
{
  // Find data shape from DATA column.
  // Make tiles of appr. 1 MB.
//  String colName (MS::columnName(MS::MODEL_DATA));
  String colName=dataManName;
  IPosition shape = ROTableColumn(ms, MS::columnName(MS::DATA)).shapeColumn();
  IPosition dataTileShape;
  if (shape.empty()) 
  {
    dataTileShape = IPosition(3, 4, nchan, (1024*1024)/(4*64*8));
  } 
  else
  {
    dataTileShape = IPosition(3, shape[0], shape[1], (1024*1024)/(shape.product()*8));
  }
  
  if (! ms.tableDesc().isColumn(colName)) 
  {
    TableDesc td;
    if (shape.empty())
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "model data"));
    }
    else
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "model data", shape, ColumnDesc::FixedShape));
    }
    TiledColumnStMan stMan(dataManName, dataTileShape);
    ms.addColumn (td, stMan);
    // Set MODEL_DATA keyword for casa::VisSet.
    // Sort out the channel selection.
    MSSpWindowColumns msSpW(ms.spectralWindow());
    Matrix<Int> selection(2, msSpW.nrow());
    // Fill in default selection (all bands and channels).
    selection.row(0) = 0;    //start
    selection.row(1) = msSpW.numChan().getColumn(); 
    ArrayColumn<Complex> mcd(ms, colName);
    mcd.rwKeywordSet().define ("CHANNEL_SELECTION",selection);
  }
}

string createColumnName(const string &ModelFilename)
{
  string columnName;
  string patchName;
  casa::String Filename;
  casa::Path Path(ModelFilename);             // casa Path object to allow basename stripping
    
  Filename=Path.baseName();                   // remove path from ModelFilename
  unsigned long pos=string::npos;             // remove .image or .img extension from Patchname  
  if((pos=Filename.find(".model")) != string::npos) // only accept .model extension anymore
  {
  }
  
  if(pos!=string::npos)                       // if we have a file suffix
  {
    patchName=Filename.substr(0, pos);        // remove it
  }
  columnName+=patchName;                      // create complete column name according to scheme

  return columnName;
}


// Helper function to print baselines[3]
//
void printBaselines(double **baselines, size_t nrows)
{
  cout << "Baselines: u, v, w" << endl;
  for(unsigned int i=0; i<nrows; i++)
  {
    cout << baselines[i][0] << ", " << baselines[i][1] << ", " << baselines[i][2] << endl;
  }
}

void printBaselines(double *u, double *v, double *w, size_t nrows)
{
  cout << "Baselines: u, v, w" << endl;
  for(unsigned int i=0; i<nrows; i++)
  {
    cout << u[i] << ", " << v[i] << ", " << w[i] << endl;
  }
}