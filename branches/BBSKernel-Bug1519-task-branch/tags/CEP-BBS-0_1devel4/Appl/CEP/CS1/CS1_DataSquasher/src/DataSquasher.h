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
#ifndef LOFARDATASQUASHER_H
#define LOFARDATASQUASHER_H

/**
@author Adriaan Renting
*/
#include <iostream>
#include <cstdlib>
#include <string>
#include <ms/MeasurementSets.h>
#include <casa/Arrays.h>
#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>

namespace LOFAR
{
  namespace CS1
  {
    using namespace casa;

    class DataSquasher
    {
    private:
      int itsNumSamples;
      int itsNumChannels;

      TableIterator CreateDataIterator(MeasurementSet& myMS);
      void GetMSInfo(MeasurementSet& myMS);
      void SquashData(Matrix<Complex>& oldData, Matrix<Complex>& newData,
                             Matrix<Bool>& oldFlags, Matrix<Bool>& newFlags,
                             int Start, int Step, int NChan, float threshold);

    public:
      DataSquasher(void);
      ~DataSquasher(void);
      int itsNumBands;
      int itsNumPolarizations;

      void TableResize(TableDesc tdesc, IPosition ipos, std::string name, Table& table);
      void Squash(MeasurementSet& inMS, MeasurementSet& outMS, std::string Data,
                  int Start, int Step, int NChan, float threshold,
                  bool UseFlags, Cube<Bool>& newFlags);
    };
  } //namespace CS1_Squasher
}; //namespace LOFAR
#endif
