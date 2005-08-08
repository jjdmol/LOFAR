//#  -*- mode: c++ -*-
//#  CalibrationAlgorithm.h: class definition for the Beam Server task.
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

#ifndef CALIBRATIONALGORITHM_H_
#define CALIBRATIONALGORITHM_H_

#include "CalibrationInterface.h"
#include "Source.h"
#include "DipoleModel.h"

namespace LOFAR {
  namespace CAL {

    class CalibrationAlgorithm : public CalibrationInterface
    {
    public:
      CalibrationAlgorithm(const Sources& sources, DipoleModels& dipolemodels);
      virtual ~CalibrationAlgorithm();
      
      virtual const Sources&      getSources()      { return m_sources;      }
      virtual       DipoleModels& getDipoleModels() { return m_dipolemodels; }
      
    private:
      const Sources&      m_sources;
            DipoleModels& m_dipolemodels;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* CALIBRATIONALGORITHM_H_ */

