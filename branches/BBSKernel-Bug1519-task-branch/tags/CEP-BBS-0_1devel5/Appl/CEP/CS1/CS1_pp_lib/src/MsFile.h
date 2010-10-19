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
#ifndef __CS1_PP_MS_FILE_H__
#define __CS1_PP_MS_FILE_H__

#include <ms/MeasurementSets.h>
#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>
#include "MsInfo.h"
#include "DataBuffer.h"

namespace LOFAR
{
  namespace CS1
  {
    //Foreward declaration
    class MsInfo;
    class RunDetails;

    class MsFile
    {
    public:
      MsFile(const std::string& msin, const std::string& msout);
      ~MsFile();

      casa::TableIterator ReadIterator();
      casa::TableIterator WriteIterator();
      void Init(MsInfo& Info, RunDetails& Details);
      void PrintInfo(void);
      void UpdateTimeslotData(casa::TableIterator& Data_iter,
                              MsInfo& Info,
                              DataBuffer& Buffer);
      void WriteData(casa::TableIterator& Data_iter,
                     MsInfo& Info,
                     DataBuffer& Buffer);


    protected:
    private:
      void TableResize(casa::TableDesc tdesc,
                       casa::IPosition ipos,
                       std::string name,
                       casa::Table& table);

      std::string InName;
      std::string OutName;
      casa::MeasurementSet* InMS;
      casa::MeasurementSet* OutMS;
    }; // class MsFile
  }; // CS1
}; // namespace LOFAR

#endif // __CS1_PP_MS_FILE_H__
