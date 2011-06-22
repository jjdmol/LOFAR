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
// MODEL_DATA_temp, and will be restored back to MODEL_DATA after the predictions.
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

#include <synthesis/MeasurementEquations/Imager.h>          // casarest ft


//using namespace casa;
using namespace std;
using namespace casa;



//casa::DirectionCoordinate getPatchDirection(const string &patchName);
casa::Vector<casa::Double> getPatchDirection(const string &patchName);
void addDirectionKeyword(casa::Table LofarMS, const string &patchName);
string createColumnName(const string &);
void removeExistingColumns(const string &MSfilename, const Vector<String> &patchNames);
void usage(const char *);

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


    // DEBUG: check input parameters
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
    
    //removeExistingColumns(MSfilename, patchNames);
    
    MeasurementSet LofarMS(MSfilename, Table::Update);          // Open LOFAR MS read/write
 
    // Keep the existing MODEL_DATA column in MODEL_DATA_temp
    //
    if(LofarMS.canRenameColumn("MODEL_DATA"))
    {
        LofarMS.renameColumn ("MODEL_DATA_temp", "MODEL_DATA"); 
    }
    else
    {
        // Recreate MODEL_DATA column
        ColumnDesc ModelColumn(ArrayColumnDesc<Complex>("MODEL_DATA"));
        LofarMS.addColumn(ModelColumn);
    }

 
    // Casarest imager object which has ft method
    Imager imager(LofarMS, casa::True, casa::True);       // create an Imager object needed for predict with ft
    Bool incremental=False;                               // create incremental UV data from models NO!    
 
    // Loop over patchNames
    for(int i=0; i < argc-2; i++)
    {
        string columnName;              // columnName for uv data of this patch in table
        Vector<String> model(1);        // we need a ft per model to write to each column
        
        //model[0]=patchNames[i];                 
        columnName=createColumnName(patchNames[i]);
        
        // DEBUG
        cout << "model = " << patchNames[i] << endl;            // DEBUG
        cout << "columnName = " << columnName << endl;          // DEBUG
    
        // Do a predict with the casarest ft() function, complist="", because we only use the model images
        imager.ft(model, "", incremental);
        
        // rename MODEL_DATA column to MODEL_DATA_patchname column
        casa::Table LofarTable(MSfilename, casa::Table::Update);  
        LofarTable.renameColumn (columnName, "MODEL_DATA");    
        addDirectionKeyword(LofarMS, patchNames[i]);           
    
        // recreate MODEL_DATA column (must be present)
        ColumnDesc ModelColumn(ArrayColumnDesc<Complex>("MODEL_DATA"));
        LofarTable.addColumn(ModelColumn);
        LofarTable.flush();
    }
    
    
    // Rename temporary MODEL_DATA_temp column back to MODEL_DATA, if necessary
    if(LofarMS.canRenameColumn("MODEL_DATA_tmep"))
    {
        LofarMS.renameColumn ("MODEL_DATA", "MODEL_DATA_temp"); 
    }
    
    //-------------------------------------------------------------------------------
    // Cleanup
    //
    LofarMS.flush();
    LofarMS.closeSubTables();                            // close Lofar MS
}


// Get the patch direction, i.e. RA/Dec of the central image pixel
//
//casa::DirectionCoordinate::DirectionCoordinate getPatchDirection(const string &patchName)
casa::Vector<casa::Double> getPatchDirection(const string &patchName)
{
    casa::IPosition imageShape;                             // shape of image
    casa::Vector<casa::Double> Pixel(2);                    // pixel coords vector of image centre
    casa::Vector<casa::Double> World(2);                    // world coords vector of image centre
    casa::PagedImage<casa::Float> image(patchName);         // open image
    
    imageShape=image.shape();                               // get centre pixel
    Pixel[0]=floor(imageShape[0]/2);
    Pixel[1]=floor(imageShape[1]/2);

    // Determine DirectionCoordinate
    casa::DirectionCoordinate dir(image.coordinates().directionCoordinate (image.coordinates().findCoordinate(casa::Coordinate::DIRECTION)));
    dir.toWorld(World, Pixel);

    return World;
}


// Add keyword "DIRECTION" containing a 2-valued vector of type double with J2000 RA and DEC of patch center in radians.
// read patch center from model MS/FIELD column PHASE_DIR
//casa::Table ModelMSField(MSfilename + "/FIELD");
//
void addDirectionKeyword(casa::Table LofarTable, const string &patchName)
{  
    casa::Vector<casa::Double> direction;
    string columnName=createColumnName(patchName);
  
    // write it to the columnDesc
    casa::TableColumn LofarModelColumn(LofarTable, columnName);
    casa::TableRecord &Model_keywords = LofarModelColumn.rwKeywordSet();
    
    direction=getPatchDirection(patchName);
   
    Model_keywords.define("Ra", direction[0]);
    Model_keywords.define("Dec", direction[1]);
}


// Create a column name based on the Modelfilename of the form
// MODEL_DATA_modelfilename (minus any file extension)
//
string createColumnName(const string &ModelFilename)
{
    string columnName="MODEL_DATA_";
    string patchName;
    unsigned long pos=ModelFilename.find(".");       // remove .image or .img extension from Patchname

    if(pos!=string::npos)                       // if we have a file suffix
    {
        patchName=ModelFilename.substr(0, pos); // remove it
    }
    columnName+=patchName.substr(0,pos);        // create complete column name according to scheme

    return columnName;
}


// Check table for existing patch names, if any of the existing patch names are already
// present in MODEL_DATA_patchname columns, offer overwrite option
//
void removeExistingColumns(const string &MSfilename, const Vector<String> &patchNames)
{
    casa::Table LofarTable(MSfilename, casa::Table::Update);     

    // First rename MODEL_DATA column to MODEL_DATA_temp in case it already contains data   
    //
    if(LofarTable.canRemoveColumn("MODEL_DATA_temp"))
    {
        LofarTable.removeColumn("MODEL_DATA_temp");
    }

    // Remove existing Patchnames
    //
    for(unsigned int i=0; i < patchNames.size(); i++)      
    {
        if(LofarTable.canRemoveColumn(patchNames[i]))
        {
            LofarTable.removeColumn(patchNames[i]);
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
