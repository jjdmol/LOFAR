/**************************************************************************
 *   Copyright (C) 2006 by ASTRON, Adriaan Renting                         *
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
#ifndef __CORRECTOR_BANDPASSCORRECTOR_H__
#define __CORRECTOR_BANDPASSCORRECTOR_H__

#include <casa/Arrays.h>
#include <utility>
#include <vector>
#include <list>
#include <map>
#include "MS_File.h"

namespace LOFAR
{
  namespace CS1
  {
    using casa::Cube;
    using casa::Matrix;
    using casa::Complex;
    using casa::Int;
    using casa::Double;
    using casa::Bool;
    using casa::Table;
    using std::list;
    using std::vector;

    typedef pair<int, int> pairii;

    class BandpassCorrector
    {
      public:
        BandpassCorrector(MS_File* MSfile,
                          int InputWindowSize);
        ~BandpassCorrector();

        void CorrectBandpass();

      protected:
        int                       NumAntennae;
        int                       NumPairs;
        int                       NumBands;
        int                       NumChannels;
        int                       NumPolarizations;
        int                       WindowSize;
        int                       NumTimeslots;
        double                    MaxBaselineLength;
        vector<double>            BaselineLengths;
        vector<pairii>            PairsIndex;
        map<pairii, int>          BaselineIndex;
        vector< Cube<Complex> >   TimeslotData;
        vector< Matrix<Complex> > WriteData;
        vector<casa::String>      AntennaNames;
        MS_File* MSfile;

        void ComputeBaselineLengths();
        void ProcessBaselineBand(Cube<Complex>* Timeslots,
                                 Matrix<Complex>* Data,
                                 int Position);
        void ProcessTimeslot(casa::TableIterator* write_iter,
                             casa::TableIterator* data_iter,
                             int Position);
        bool UpdateTimeslotData(vector<int>* OldFields,
                                vector<int>* OldBands,
                                int* TimeCounter,
                                Table* TimeslotTable,
                                double* Time);
      private:
    }; // BandpassCorrector
  }; // namespace CS1
}; // namespace LOFAR

#endif //  __CORRECTOR_BANDPASSCORRECTOR_H__
