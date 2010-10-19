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
#ifndef __SQUASHER_MS_READER_H__
#define __SQUASHER_MS_READER_H__

#include <ms/MeasurementSets.h>
#include <vector>
#include <string>
#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>

namespace WSRT
{
  class MS_Reader
  {
  public:
     MS_Reader(const std::string& msname);
    ~MS_Reader();

    int                     itsNumSamples;
    int                     itsNumAntennae;
    int                     itsNumFields;
    int                     itsNumBands;
    int                     itsNumChannels;
    int                     itsNumPolarizations;
    int                     itsNumPairs;
    int                     itsNumTimeslots;
    double                  itsNoiseLevel;
    std::vector<casa::String> itsAntennaNames;
    casa::Vector<casa::Int>   itsPolarizations;

    casa::TableIterator TimeslotIterator();
    casa::TableIterator TimeAntennaIterator();
    casa::MSAntenna     antenna();
    void                WriteDataPointFlags(casa::TableIterator* flag_iter,
                                            casa::Matrix<casa::Bool>* Flags,
                                            bool FlagcompleteRow,
                                            bool ExistingFlags);

  protected:
    string                MSName;
    casa::MeasurementSet* MS;
    void init();
  private:
  }; // MS_Reader
}; // namespace WSRT

#endif // __SQUASHER_MS_READER_H__
