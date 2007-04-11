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

    void DataSquasher::GetMSInfo(MeasurementSet& myMS)
    {
      //Number of channels in the Band
      MSSpectralWindow spectral_window = myMS.spectralWindow();
      ROScalarColumn<Int>              NUM_CHAN_col(spectral_window, "NUM_CHAN");
      itsNumChannels                   = NUM_CHAN_col(0);

      //Number of polarizations
      MSPolarization polarization      = myMS.polarization();
      ROScalarColumn<Int>              NUM_CORR_col(polarization, "NUM_CORR");
      itsNumPolarizations              = NUM_CORR_col(0);
    }

    //===============>>>  DataSquasher::CreateDataIterator  <<<===============

    TableIterator DataSquasher::CreateDataIterator(MeasurementSet& myMS)
    {
      Block<String> ms_iteration_variables(4);
      ms_iteration_variables[0] = "TIME_CENTROID";
      ms_iteration_variables[1] = "DATA_DESC_ID";
      ms_iteration_variables[2] = "ANTENNA1";
      ms_iteration_variables[3] = "ANTENNA2";

      return TableIterator(myMS, ms_iteration_variables);
    }

    //===============>>>  DataSquasher::SquashData  <<<===============

    void DataSquasher::SquashData(Matrix<Complex>& oldData, Matrix<Complex>& newData, int Start, int Step, int NChan)
    {
      int incounter  = 0;
      int outcounter = 0;
      Vector<Complex> values(itsNumPolarizations, 0);
      while (incounter < NChan)
      {
        for (int j = 0; j < itsNumPolarizations; j++)
        {
          values(j) += oldData(j, Start + incounter);
        }
        incounter++;
        if ((incounter) % Step == 0)
        {
          newData.column(outcounter) = values;
          values = 0;
          outcounter++;
        }
      }
    }

    //===============>>>  DataSquasher::Squash  <<<===============

    void DataSquasher::Squash(MeasurementSet& myMS, std::string OldData, std::string NewData,
                              int Start, int Step, int NChan)
    {
      TableIterator iter = CreateDataIterator(myMS);
      GetMSInfo(myMS);
      int step = myMS.nrow() / 10 + 1; //not exact but it'll do
      int row  = 0;
      while (!iter.pastEnd())
      {
        if (row++ % step == 0) // to tell the user how much % we have processed,
        { std::cout << 10*(row/step) << "%" << std::endl; //not very accurate for low numbers of timeslots
        }
        Table         DataTable = iter.table();
        ROArrayColumn<Complex> Old(DataTable, OldData);
        ArrayColumn<Complex>   New(DataTable, NewData);
        Matrix<Complex>        myOldData(itsNumPolarizations, itsNumChannels);
        Matrix<Complex>        myNewData(itsNumPolarizations, NChan/Step);

        Old.get(0, myOldData);
        SquashData(myOldData, myNewData, Start, Step, NChan);
        New.put(0, myNewData);
        iter++;
      }
    }
  } //namespace CS1
}; //namespace LOFAR
