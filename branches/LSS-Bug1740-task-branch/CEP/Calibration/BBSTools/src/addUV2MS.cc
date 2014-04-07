//# addUV2MS: adds uv data from a (casa) MS file to a named column of another
//# (LOFAR) MS
//#
//# Copyright (C) 2011
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
//# $Id: addUV2MS.h xxxx 2011-05-16 12:59:37Z duscha $

// \file takes a LOFAR MS, using its UV-coverage and bandwidth coverage
// to predict uv-data from (CASA) input image(s)
// Internally casarest's ft() task is used to predict the uv-data. The uv
// data is a MODEL_DATA_<patchname> column (where the file extension has been
// stripped off). If multiple images are supplied, multiple columns are being
// created.
//
// Any existing MODEL_DATA column is copied over to a temporary column,
// MODEL_DATA_bak, and will be restored back to MODEL_DATA after the predictions.
//
// If MODEL_DATA_<patchname> columns are already present, overwrite or abort is
// offered.

#include <lofar_config.h>

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ArrayColumnFunc.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/VectorSTLIterator.h>
#include <math.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>    // DirectionCoordinate needed for patch direction
#include <images/Images/PagedImage.h>                       // we need to open the image to determine patch centre direction
#include <synthesis/MeasurementEquations/Imager.h>          // casarest ft()

#include <tables/Tables/TiledColumnStMan.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

//using namespace casa;
using namespace std;
using namespace casa;



//casa::DirectionCoordinate getPatchDirection(const string &patchName);
//--------------------------------------------------------------
// Function declarations (helper functions)
//
casa::MDirection getPatchDirection(const string &patchName);
void addDirectionKeyword(casa::Table LofarMS, const string &patchName);
void addChannelSelectionKeyword(Table &LofarTable, const string &columnName);
string createColumnName(const casa::String &);
void removeExistingColumns(const string &MSfilename, const Vector<String> &patchNames);
void addImagerColumns (MeasurementSet& ms);
void addModelColumn (MeasurementSet& ms, const String& dataManName);

//--------------------------------------------------------------
// Function declarations (debug functions)
//
Vector<String> getColumnNames(const Table &table);
void showColumnNames(Table &table);

void usage(const char *);


//----------------------------------------------------------------
// Main
//
int main(int argc, char *argv[])
{
  casa::String MSfilename;        // Filename of LafarMS
  Vector<String> patchNames;      // vector with filenames of patches used as models
    
  if(argc < 3)        // if not enough parameters given, display usage information
  {
      usage(argv[0]);
      exit(0);
  }
  else
  {
      MSfilename=argv[1];
      for(int i=2; i < argc; i++)
      {
          Int length;                          // Shape, i.e. length of Vector
          patchNames.shape(length);            // determine length of Vector    
          patchNames.resize(length+1, True);   // resize and copy values
          patchNames[length]=argv[i];
      }    
  }
  if(MSfilename=="")
  {
      casa::AbortError("No MS filename given");         // raise exception
      exit(0);
  }
  if(patchNames.size()==0)
  {
      casa::AbortError("No patch image filename given");
      exit(0);
  }
    
  //-------------------------------------------------------------------------------
  // Do the work    
    
  // remove existing MODEL_DATA_<patch> columns (only for patches supplied as 
  // parameter, other columns will be left untouched)
  removeExistingColumns(MSfilename, patchNames);    
  MeasurementSet LofarMS(MSfilename, Table::Update);          // Open LOFAR MS read/write

  // In case a previous run failed, MODEL_DATA-bak may still exist
  // and should be renamed to MODEL_DATA. An existing MODEL_DATA column
  // is for a patch and should be removed.
  if(LofarMS.tableDesc().isColumn("MODEL_DATA_bak"))
  {
      if(LofarMS.tableDesc().isColumn("MODEL_DATA"))
      {
	      LofarMS.removeColumn("MODEL_DATA");
      }
      LofarMS.renameColumn("MODEL_DATA", "MODEL_DATA_bak");
  }

  // Make sure the imager columns exist for the CASA ft function.
  addImagerColumns (LofarMS);
  LofarMS.flush();

  // Rename the existing MODEL_DATA.
  if(LofarMS.tableDesc().isColumn("MODEL_DATA"))
  {
      LofarMS.renameColumn ("MODEL_DATA_bak", "MODEL_DATA");
  }
 
  // Loop over patchNames
  for(unsigned int i=0; i < patchNames.size(); i++)
    {
      string columnName;              // columnName for uv data of this patch in table
      Vector<String> model(1);        // we need a ft per model to write to each column

      columnName=createColumnName(patchNames[i]);       
      model[0]=patchNames[i];

      // Add the MODEL_DATA column for this patch.
      addModelColumn(LofarMS, columnName);
      LofarMS.flush();

      //showColumnNames(LofarMS);
      // Do a predict with the casarest ft() function, complist="", because we only use the model images
      // Casarest Imager object which has ft method
      // (last parameter casa::True "use MODEL_DATA column")
      Imager imager(LofarMS, True, True);
      imager.ft(model, "", False);
       
      // rename MODEL_DATA column to MODEL_DATA_patchname column
      LofarMS.renameColumn (columnName, "MODEL_DATA");
        
      addDirectionKeyword(LofarMS, patchNames[i]);           
       
      LofarMS.flush();
    }
    
    
  // Rename MODEL_DATA_bak column back to MODEL_DATA.
  if(LofarMS.canRenameColumn("MODEL_DATA_bak"))
  {
    LofarMS.renameColumn ("MODEL_DATA", "MODEL_DATA_bak"); 
  }
    
  //-------------------------------------------------------------------------------
  // Cleanup
  //
  LofarMS.flush();
  LofarMS.closeSubTables();                            // close Lofar MS
}



// Get the patch direction, i.e. RA/Dec of the central image pixel
//
casa::MDirection getPatchDirection(const string &patchName)
{
  casa::IPosition imageShape;                             // shape of image
  casa::Vector<casa::Double> Pixel(2);                    // pixel coords vector of image centre
  casa::MDirection MDirWorld(casa::MDirection::J2000);   // astronomical direction in J2000
  casa::PagedImage<casa::Float> image(patchName);         // open image
    
  imageShape=image.shape();                               // get centre pixel
  Pixel[0]=floor(imageShape[0]/2);
  Pixel[1]=floor(imageShape[1]/2);

  // Determine DirectionCoordinate
  casa::DirectionCoordinate dir(image.coordinates().directionCoordinate (image.coordinates().findCoordinate(casa::Coordinate::DIRECTION)));
  dir.toWorld(MDirWorld, Pixel);

  return MDirWorld;
}


// Add keyword "LOFAR_DIRECTION" containing a 2-valued vector of type double with J2000 RA and DEC of patch 
// center in radians.
//
void addDirectionKeyword(casa::Table LofarTable, const string &patchName)
{  
  //casa::Vector<casa::Double> direction;
  casa::MDirection direction(casa::MDirection::J2000);
  string columnName=createColumnName(patchName);
  
  // write it to the columnDesc
  casa::TableColumn LofarModelColumn(LofarTable, columnName);
  casa::TableRecord &Model_keywords = LofarModelColumn.rwKeywordSet();
  casa::TableRecord MDirectionRecord;
  casa::String error="";

  direction=getPatchDirection(patchName);
  
  // Use MeasureHolder class to convert MDirection to a keyword-compatible format
  casa::MeasureHolder mHolder(direction);
  if(!mHolder.toRecord(error, MDirectionRecord))
  {
    casa::AbortError("addDirectionKeyword()");
  }
  else
  {
    Model_keywords.defineRecord("LOFAR_DIRECTION", MDirectionRecord);
  }
}


// Create a column name based on the Modelfilename of the form
// modelfilename (minus any file extension)
//
string createColumnName(const casa::String &ModelFilename)
{
  string columnName;
  string patchName;
  casa::String Filename;
  casa::Path Path(ModelFilename);             // casa Path object to allow basename stripping
    
  Filename=Path.baseName();                   // remove path from ModelFilename
  unsigned long pos=Filename.find(".");       // remove .image or .img extension from Patchname

  if(pos!=string::npos)                       // if we have a file suffix
  {
    patchName=Filename.substr(0, pos);        // remove it
  }
  columnName+=patchName;                      // create complete column name according to scheme

  return columnName;
}


// Check table for existing patch names, if any of the existing patch names are already
// present in <patchname> columns, offer overwrite option
//
void removeExistingColumns(const string &MSfilename, const Vector<String> &patchNames)
{
  string columnName;
  casa::Table LofarTable(MSfilename, casa::Table::Update);     

  // Remove existing Patchnames, but only those that are specified in the list of current run
  // it preserves other patch columns
  //
  for(unsigned int i=0; i < patchNames.size(); i++)      
  {
    columnName=createColumnName(patchNames[i]);        
    if(LofarTable.tableDesc().isColumn(columnName))
    {
      LofarTable.removeColumn(columnName);
    }
  }
  LofarTable.flush();
  LofarTable.closeSubTables();
}

// Display usage info
//
void usage(const char *programname)
{
  cout << "Usage: " << programname << ": LofarMS <patchname[s]>" << endl;
  cout << "LofarMS        - MS to add model data to" << endl;
  cout << "<patchname[s]> - list of patchname[s] of image[s], these filenames are used to name the column and should be" << endl;
  cout << "                 referred to in the skymodel file with .MS extension removed" << endl;
}


// From: CEP/MS/src/MSCreate::addImagerColumns
//
void addImagerColumns (MeasurementSet& ms)
{
  // Find data shape from DATA column.
  // Make tiles of appr. 1 MB.
  IPosition shape = ROTableColumn(ms, MS::columnName(MS::DATA)).shapeColumn();
  IPosition dataTileShape;
  if (shape.empty()) 
  {
    dataTileShape = IPosition(3, 4, 64, (1024*1024)/(4*64*8));
  }
  else
  {
    dataTileShape = IPosition(3, shape[0], shape[1], (1024*1024)/(shape.product()*8));
  }
  String colName = MS::columnName(MS::CORRECTED_DATA);
  if (! ms.tableDesc().isColumn(colName)) 
  {
    TableDesc td;
    if (shape.empty()) 
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "corrected data"));
    }
    else
    {
      td.addColumn (ArrayColumnDesc<Complex>(colName, "corrected data", shape,
                                             ColumnDesc::FixedShape));
    }
    TiledColumnStMan stMan("TiledCorrectedData", dataTileShape);
    ms.addColumn (td, stMan);
  }
  ////  addModelColumn (ms, "TiledModelData");
  colName = MS::columnName(MS::IMAGING_WEIGHT);
  if (! ms.tableDesc().isColumn(colName)) 
  {
    TableDesc td;
    if (shape.empty())
    {
      td.addColumn (ArrayColumnDesc<Float>(colName, "imaging weight"));
    }
    else
    {
      td.addColumn (ArrayColumnDesc<Float>(colName, "imaging weight",
                                           IPosition(1, shape[1]),
                                           ColumnDesc::FixedShape));
    }
    TiledColumnStMan stMan("TiledImagingWeight", dataTileShape.getLast(2));
    ms.addColumn (td, stMan);
  }
}

void addModelColumn (MeasurementSet& ms, const String& dataManName)
{
  // Find data shape from DATA column.
  // Make tiles of appr. 1 MB.
  String colName (MS::columnName(MS::MODEL_DATA));
  IPosition shape = ROTableColumn(ms, MS::columnName(MS::DATA)).shapeColumn();
  IPosition dataTileShape;
  if (shape.empty()) 
  {
    dataTileShape = IPosition(3, 4, 64, (1024*1024)/(4*64*8));
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




//------------------------------------------------------------------
// DEBUG functions

// Get the column names
//
Vector<String> getColumnNames(const Table &table)
{
  TableDesc td=table.tableDesc();
  return td.columnNames();
}

// Display the column Names of the table
//
void showColumnNames(Table &table)
{
  Vector<String> columnNames; 
  columnNames=getColumnNames(table);
    
  for(Vector<String>::iterator it=columnNames.begin(); it!=columnNames.end(); ++it)
  {
    cout << *it << "\t";
  }
  cout << endl;
}
