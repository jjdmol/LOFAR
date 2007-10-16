/***************************************************************************
 *   Copyright (C) 2007 by Adriaan Renting, ASTRON                         *
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
//#include <casa/Arrays/ArrayLogical.h>

#include "DataSquasher.h"
namespace LOFAR
{
  namespace CS1
  {
    using namespace casa;

    //===============>>>  DataSquasher::DataSquasher  <<<===============

    DataSquasher::DataSquasher(void)
    {
    }

    //===============>>>  DataSquasher::~DataSquasher  <<<===============

    DataSquasher::~DataSquasher(void)
    {
    }

    //===============>>>  DataSquasher::GetMSInfo  <<<===============

    void DataSquasher::GetMSInfo(MeasurementSet& outMS)
    {
      //Number of samples
      itsNumSamples                    = outMS.nrow();

      //Number of channels in the Band
      MSSpectralWindow spectral_window = outMS.spectralWindow();
      ROScalarColumn<Int>              NUM_CHAN_col(spectral_window, "NUM_CHAN");
      itsNumChannels                   = NUM_CHAN_col(0);

      //Number of Bands
      itsNumBands                      = NUM_CHAN_col.nrow();

      //Number of polarizations
      MSPolarization polarization      = outMS.polarization();
      ROScalarColumn<Int>              NUM_CORR_col(polarization, "NUM_CORR");
      itsNumPolarizations              = NUM_CORR_col(0);
    }

    //===============>>>  DataSquasher::CreateDataIterator  <<<===============

    TableIterator DataSquasher::CreateDataIterator(MeasurementSet& outMS)
    {
      Block<String> ms_OutIteration_variables(4);
      ms_OutIteration_variables[0] = "TIME_CENTROID";
      ms_OutIteration_variables[1] = "DATA_DESC_ID";
      ms_OutIteration_variables[2] = "ANTENNA1";
      ms_OutIteration_variables[3] = "ANTENNA2";

      return TableIterator(outMS, ms_OutIteration_variables);
    }

    //===============>>>  DataSquasher::TableResize  <<<===============

    void DataSquasher::TableResize(TableDesc tdesc, IPosition ipos, string name, Table& table)
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

    //===============>>>  DataSquasher::SquashData  <<<===============

    void DataSquasher::SquashData(Matrix<Complex>& oldData, Matrix<Complex>& newData,
                                  Matrix<Bool>& oldFlags, Matrix<Bool>& newFlags,
                                  int Start, int Step, int NChan, float threshold)
    {
      int incounter  = 0;
      int outcounter = 0;
      bool flagrow   = true;
      Vector<Complex> values(itsNumPolarizations, 0);
      Vector<Complex> weights(itsNumPolarizations, 0);
      while (incounter < NChan)
      {
        for (int i = 0; i < itsNumPolarizations; i++)
        {
          if (!oldFlags(i, Start + incounter))
          { //weight is not handled here, maybe sometime in the future?
            values(i) += oldData(i, Start + incounter);
            weights(i) += 1;
            flagrow = false;
          }
        }
        incounter++;
        if ((incounter) % Step == 0)
        {
          for (int i = 0; i < itsNumPolarizations; i++)
          { values(i) = values(i) / weights(i);
          }
          newData.column(outcounter)  = values;
          for (int i = 0; i < itsNumPolarizations; i++) // I can't get anyGT or something like that to work
          flagrow = flagrow || (threshold * 1.001 >= abs(values[i]));
          newFlags.column(outcounter) = flagrow;
          values = 0;
          weights = 0;
          outcounter++;
          flagrow = true;
        }
      }
    }

    //===============>>>  DataSquasher::Squash  <<<===============

    void DataSquasher::Squash(MeasurementSet& inMS, MeasurementSet& outMS, std::string Data,
                              int Start, int Step, int NChan, float threshold,
                              bool UseFlags, Cube<Bool>& newFlags)
    {
      TableIterator inIter  = CreateDataIterator(inMS);
      TableIterator outIter = CreateDataIterator(outMS);
      GetMSInfo(outMS);
      int step = outMS.nrow() / 10 + 1; //not exact but it'll do
      int row  = 0;
      bool rwFlags = newFlags.nrow() > 0;
      if (!rwFlags) //we are able to re-use the same flags
      { newFlags.resize(itsNumPolarizations, NChan/Step, itsNumSamples);
      }

      while (!outIter.pastEnd())
      {
        if (row++ % step == 0) // to tell the user how much % we have processed,
        { std::cout << 10*(row/step) << "%" << std::endl; //not very accurate for low numbers of timeslots
        }
        Table         InDataTable  = inIter.table();
        Table         OutDataTable = outIter.table();
        ROArrayColumn<Complex> Old(InDataTable, Data);
        ArrayColumn<Complex>   New(OutDataTable, Data);
        Matrix<Complex>        myOldData(itsNumPolarizations, itsNumChannels);
        Matrix<Complex>        myNewData(itsNumPolarizations, NChan/Step);

        ROArrayColumn<Bool>    Flags(OutDataTable, "FLAG");
        Matrix<Bool>           myOldFlags(itsNumPolarizations, itsNumChannels, false);
        Matrix<Bool>           myNewFlags(itsNumPolarizations, NChan/Step);

        Old.get(0, myOldData);
        if (UseFlags)
        { Flags.get(0, myOldFlags);
        }
        SquashData(myOldData, myNewData, myOldFlags, myNewFlags, Start, Step, NChan, threshold);
        New.put(0, myNewData);
        if (!rwFlags)
        { newFlags.xyPlane(row - 1) = myNewFlags;
        }
        outIter++;
        inIter++;
      }
    }
  } //namespace CS1
}; //namespace LOFAR
