//#  -*- mode: c++ -*-
//#  DipoleModelData.h: declaration of the DipoleModelData class
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

#ifndef DIPOLEMODELDATA_H_
#define DIPOLEMODELDATA_H_

#include <complex>
#include <blitz/array.h>
#include <fstream>

namespace LOFAR {
  namespace CAL {

    class DipoleModelData
    {
    public:
      DipoleModelData();
      virtual ~DipoleModelData();

      bool getNextFromFile(std::string filename);

      std::string getName() const { return m_name; }
      const blitz::Array<std::complex<double>, 4>& getSens() const { return m_sens; }

    private:
      std::string   m_filename;
      std::ifstream m_file;

      std::string                           m_name;
      blitz::Array<std::complex<double>, 4> m_sens;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* DIPOLEMODELDATA_H_ */

