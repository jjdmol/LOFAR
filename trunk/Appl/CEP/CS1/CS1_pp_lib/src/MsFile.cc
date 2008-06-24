/***************************************************************************
 *   Copyright (C) 2007-8 by ASTRON, Adriaan Renting                       *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <casa/BasicMath/Math.h>
#include <casa/Arrays.h>

#include <iostream>

#include "MsFile.h"
#include "MsInfo.h"
#include "RunDetails.h"

using namespace LOFAR::CS1;
using namespace casa;



//===============>>>  MsFile::MsFile  <<<===============

MsFile::MsFile(const std::string& msin, const std::string& msout)
{
  InName  = msin;
  OutName = msout;
}

//===============>>>  MsFile::~MsFile  <<<===============

MsFile::~MsFile()
{
  delete InMS;
  delete OutMS;
}

//===============>>>  DataSquasher::TableResize  <<<===============

void MsFile::TableResize(TableDesc tdesc, IPosition ipos, string name, Table& table)
{
  ColumnDesc desc = tdesc.rwColumnDesc(name);
  desc.setOptions(0);
  desc.setShape(ipos);
  desc.setOptions(4);
  if (table.tableDesc().isColumn(name))
  { table.removeColumn(name);
  }
  table.addColumn(desc);
}

//===============>>> MsFile::PrintInfo  <<<===============
void MsFile::Init(MsInfo& Info, RunDetails& Details)
{
  cout << "Please wait, creating output MS" << endl;
  Table temptable = tableCommand(string("SELECT UVW,FLAG_CATEGORY,WEIGHT,SIGMA,ANTENNA1,ANTENNA2,ARRAY_ID,DATA_DESC_ID,") +
                                  string("EXPOSURE,FEED1,FEED2,FIELD_ID,FLAG_ROW,INTERVAL,OBSERVATION_ID,PROCESSOR_ID,") +
                                  string("SCAN_NUMBER,STATE_ID,TIME,TIME_CENTROID,FLAG FROM ") + InName);
  // NOT copying WEIGHT_SPECTRUM as it only contains dummy data anyway
  // We do need FLAG to make it a valid MS
  temptable.deepCopy(OutName, Table::NewNoReplace);

  InMS    = new MeasurementSet(InName);
  OutMS   = new MeasurementSet(OutName, Table::Update);

  //some magic to create a new DATA column
  TableDesc tdesc = InMS->tableDesc();
  ColumnDesc desc = tdesc.rwColumnDesc("DATA");
  IPosition ipos  = desc.shape();
  Vector<Int> temp_pos = ipos.asVector();
  std::cout << "Old shape: " << temp_pos(0) << ":" <<  temp_pos(1) << std::endl;
  int old_nchan = temp_pos(1);
  int new_nchan = Details.NChan/Details.Step;
  temp_pos(1)   = new_nchan;
  std::cout << "New shape: " << temp_pos(0) << ":" <<  temp_pos(1) << std::endl;
  IPosition data_ipos(temp_pos);

  //tdesc.removeColumn("WEIGHT_SPECTRUM");
  tdesc.addColumn(ArrayColumnDesc<Float>("WEIGHT_SPECTRUM", "Added by datasquasher",
                                          data_ipos, ColumnDesc::FixedShape));

  TableResize(tdesc, data_ipos, "DATA", *OutMS);
  TableResize(tdesc, data_ipos, "WEIGHT_SPECTRUM", *OutMS);

  //do the actual data squashing
//  itsSquasher->Squash(inMS, outMS, "DATA",
//                      itsStart, itsStep, itsNChan, itsThreshold,
//                      itsSkip, NewFlags);

  //if present handle the CORRECTED_DATA column
  if (tdesc.isColumn("CORRECTED_DATA"))
  {
    if (Details.Columns)
    {
      cout << "Preparing CORRECTED_DATA" << endl;
      TableResize(tdesc, data_ipos, "CORRECTED_DATA", *OutMS);

      //do the actual data squashing
//      itsSquasher->Squash(inMS, outMS, "CORRECTED_DATA",
//                          itsStart, itsStep, itsNChan, itsThreshold,
//                          itsSkip, NewFlags);
    }
    else
    { OutMS->removeColumn("CORRECTED_DATA");
    }
  }

  //if present handle the MODEL_DATA column
  if (tdesc.isColumn("MODEL_DATA"))
  {
    if (Details.Columns)
    {
      cout << "Processing MODEL_DATA" << endl;
      desc = tdesc.rwColumnDesc("MODEL_DATA");
      desc.setOptions(0);
      desc.setShape(data_ipos);
      desc.setOptions(4);
      desc.rwKeywordSet().removeField("CHANNEL_SELECTION"); //messes with the Imager if it's there but has wrong values
      Matrix<Int> selection;
      selection.resize(2, Info.NumBands); //dirty hack with direct reference to itsSquasher
      selection.row(0) = 0; //start in Imager, will therefore only work if imaging whole SPW
      selection.row(1) = new_nchan;
      desc.rwKeywordSet().define("CHANNEL_SELECTION", selection); // #spw x [startChan, NumberChan] for the VisBuf in the Imager
      // see code/msvis/implement/MSVis/VisSet.cc
      OutMS->addColumn(desc);
      OutMS->addColumn(ArrayColumnDesc<Float>("IMAGING_WEIGHT","imaging weight", 1));

      //do the actual data squashing
//      itsSquasher->Squash(inMS, outMS, "MODEL_DATA",
//                          itsStart, itsStep, itsNChan, itsThreshold,
//                          itsSkip, NewFlags);
    }
    else
    { OutMS->removeColumn("MODEL_DATA");
      OutMS->removeColumn("IMAGING_WEIGHT");
    }
  }

  //fix the FLAGS column
  TableResize(tdesc, data_ipos, "FLAG", *OutMS);

  //Fix the SpectralWindow values
  IPosition spw_ipos(1,new_nchan);
  MSSpectralWindow inSPW = InMS->spectralWindow();
  //ugly workaround MSSpectral window does no allow deleting and then recreating columns
  Table outSPW = Table(OutName + "/SPECTRAL_WINDOW", Table::Update);
  ScalarColumn<Int> channum(outSPW, "NUM_CHAN");
  channum.fillColumn(new_nchan);

  TableDesc SPWtdesc = inSPW.tableDesc();
  TableResize(SPWtdesc, spw_ipos, "CHAN_FREQ", outSPW);

  TableResize(SPWtdesc, spw_ipos, "CHAN_WIDTH", outSPW);

  TableResize(SPWtdesc, spw_ipos, "EFFECTIVE_BW", outSPW);

  TableResize(SPWtdesc, spw_ipos, "RESOLUTION", outSPW);

  ROArrayColumn<Double> inFREQ(inSPW, "CHAN_FREQ");
  ROArrayColumn<Double> inWIDTH(inSPW, "CHAN_WIDTH");
  ROArrayColumn<Double> inBW(inSPW, "EFFECTIVE_BW");
  ROArrayColumn<Double> inRESOLUTION(inSPW, "RESOLUTION");

  ArrayColumn<Double> outFREQ(outSPW, "CHAN_FREQ");
  ArrayColumn<Double> outWIDTH(outSPW, "CHAN_WIDTH");
  ArrayColumn<Double> outBW(outSPW, "EFFECTIVE_BW");
  ArrayColumn<Double> outRESOLUTION(outSPW, "RESOLUTION");

  Vector<Double> old_temp(old_nchan, 0.0);
  Vector<Double> new_temp(new_nchan, 0.0);

  for (unsigned int i = 0; i < inSPW.nrow(); i++)
  {
    for (int j = 0; j < new_nchan; j++)
    { inFREQ.get(i, old_temp);
      if (Details.Step % 2) //odd number of channels in step
      { new_temp(j) = old_temp(Details.Start + j*Details.Step + (Details.Step + 1)/2);
      }
      else //even number of channels in step
      { new_temp(j) = 0.5 * (old_temp(Details.Start + j*Details.Step + Details.Step/2 -1)
                              + old_temp(Details.Start + j*Details.Step + Details.Step/2));
      }
      outFREQ.put(i, new_temp);
    }
    for (int j = 0; j < new_nchan; j++)
    { inWIDTH.get(i, old_temp);
      new_temp(j) = old_temp(0) * Details.Step;
      outWIDTH.put(i, new_temp);
    }
    for (int j = 0; j < new_nchan; j++)
    { inBW.get(i, old_temp);
      new_temp(j) = old_temp(0) * Details.Step;
      outBW.put(i, new_temp);
    }
    for (int j = 0; j < new_nchan; j++)
    { inRESOLUTION.get(i, old_temp);
      new_temp(j) = old_temp(0) * Details.Step;
      outRESOLUTION.put(i, new_temp);
    }
  }
  cout << "Finished creating output MS" << endl;
}

//===============>>> MsFile::PrintInfo  <<<===============
void MsFile::PrintInfo(void)
{
  std::cout << "In  MeasurementSet:   " << InName << std::endl;
  std::cout << "Out MeasurementSet:   " << OutName << std::endl;
}

//===============>>> MsFile::BaselineIterator  <<<===============

TableIterator MsFile::ReadIterator()
{
  Block<String> ms_iteration_variables(1);
  ms_iteration_variables[0] = "TIME_CENTROID";

  return TableIterator((*InMS), ms_iteration_variables);
}

//===============>>> MsFile::BaselineIterator  <<<===============

TableIterator MsFile::WriteIterator()
{
  Block<String> ms_iteration_variables(1);
  ms_iteration_variables[0] = "TIME_CENTROID";

  return TableIterator((*OutMS), ms_iteration_variables);
}

//===============>>> MsFile::UpdateTimeslotData  <<<===============
void MsFile::UpdateTimeslotData(casa::TableIterator Data_iter,
                                MsInfo& Info,
                                DataBuffer& Buffer)
{
  Table         TimeslotTable = Data_iter.table();
  int           rowcount      = TimeslotTable.nrow();
  ROTableVector<Int>            antenna1(TimeslotTable, "ANTENNA1");
  ROTableVector<Int>            antenna2(TimeslotTable, "ANTENNA2");
  ROTableVector<Int>            bandnr  (TimeslotTable, "DATA_DESC_ID");
  ROArrayColumn<Complex>        data    (TimeslotTable, "DATA");
  Cube<Complex>                 tempData(Info.NumPolarizations, Info.NumChannels, rowcount);
  ROArrayColumn<Bool>           flags   (TimeslotTable, "FLAG");
  Cube<Bool>                    tempFlags(Info.NumPolarizations, Info.NumChannels, rowcount);

  data.getColumn(tempData); //We're not checking Data.nrow() Data.ncolumn(), assuming all data is the same size.
  flags.getColumn(tempFlags);

  Buffer.Position = ++(Buffer.Position) % Buffer.WindowSize;
  cout << Buffer.Position << endl;
  for (int i = 0; i < rowcount; i++)
  {
    int bi    = Info.BaselineIndex[baseline_t(antenna1(i), antenna2(i))];
    int band  = bandnr(i);
    int index = (band % Info.NumBands) * Info.NumPairs + bi;

    Buffer.Data[index].xyPlane(Buffer.Position)  = tempData.xyPlane(i);
    Buffer.Flags[index].xyPlane(Buffer.Position) = tempFlags.xyPlane(i);
  }
}

//===============>>> MsFile::WriteFlags  <<<===============

void MsFile::WriteData(casa::TableIterator Data_iter,
                       MsInfo& Info,
                       DataBuffer& Buffer)
{
  Table         DataTable = Data_iter.table();
  int           rowcount  = DataTable.nrow();
  ROTableVector<Int>        antenna1(DataTable, "ANTENNA1");
  ROTableVector<Int>        antenna2(DataTable, "ANTENNA2");
  ROTableVector<Int>        bandnr  (DataTable, "DATA_DESC_ID");
  ArrayColumn  <Complex>    data(DataTable, "DATA");
  ArrayColumn  <Bool>       flags(DataTable, "FLAG");

  for (int i = 0; i < rowcount; i++)
  {
    int bi    = Info.BaselineIndex[baseline_t(antenna1(i), antenna2(i))];
    int band  = bandnr(i);
    int index = (band % Info.NumBands) * Info.NumPairs + bi;
    data.put(i, Buffer.Data[index].xyPlane(Buffer.Position));
    flags.put(i, Buffer.Flags[index].xyPlane(Buffer.Position));
  }
}

//===============>>> MsFile  <<<===============

