//#  -*- mode: c++ -*-
//#  CalAlgorithm.h: class definition for the Beam Server task.
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

#ifndef CALALGORITHM_H_
#define CALALGORITHM_H_

#include "CalInterface.h"

namespace CAL
{
  class CalAlgorithm : public CalInterface
  {
    public:
      CalAlgorithm() {}
      virtual ~CalAlgorithm() {}
      
      virtual void setACC();
      virtual void getACC();
      virtual void setSourceCatalog();
      virtual void getSourceCatalog();
      virtual void setDipoleModel();
      virtual void getDipoleModel();
      
    private:
      int m_pos;
      int m_spw;
      int m_acc;
      int m_catalog;
      int m_sens;
  };
};

#endif /* CALALGORITHM_H_ */

