//#  -*- mode: c++ -*-
//#  SourceData.h: definition of data for a sky source
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef SOURCEDATA_H_
#define SOURCEDATA_H_

#include <string>
#include <blitz/array.h>
#include <fstream>

namespace LOFAR {
  namespace CAL {

    class SourceData
    {
    public:

      SourceData();
      virtual ~SourceData();

      bool getNextFromFile(std::string filename);

      std::string getName() const { return m_name; }
      double getRA() const { return m_ra; }
      double getDEC() const { return m_dec; }
      const blitz::Array<double, 2>& getFlux() const { return m_flux; }

    private:
      std::string             m_filename;
      std::ifstream           m_file;

      std::string             m_name;
      double                  m_ra;
      double                  m_dec;
      blitz::Array<double, 2> m_flux;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* SOURCEDATA_H_ */

