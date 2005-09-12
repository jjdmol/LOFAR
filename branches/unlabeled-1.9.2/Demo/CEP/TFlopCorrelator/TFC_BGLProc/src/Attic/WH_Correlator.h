//#  WH_Correlator.h: correlator
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

#ifndef BGL_WH_CORRELATOR_H
#define BGL_WH_CORRELATOR_H

//# Includes
#include <Common/Timer.h>
#include <APS/ParameterSet.h>
#include <tinyCEP/WorkHolder.h>

#include <TFC_Interface/DH_CorrCube.h>
#include <TFC_Interface/DH_Vis.h>

/* #define NR_STATIONS			 37 */
/* #define NR_POLARIZATIONS		  2 */
/* #define NR_SUB_CHANNELS			256 */
/* #define NR_CORRELATORS_PER_FILTER	  5 */

/* #define NR_CHANNELS_PER_CORRELATOR ((NR_SUB_CHANNELS - 1) / NR_CORRELATORS_PER_FILTER) */

namespace LOFAR
{
  
  class WH_Correlator: public WorkHolder {

  public:
    explicit WH_Correlator (const string& name, int nchannels);
    virtual ~WH_Correlator();
    
    virtual WorkHolder* construct(const string& name, int nchannels);
    virtual WH_Correlator* make(const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void dump() const;
    virtual void postprocess();

  private:
    /// forbid copy constructor
    WH_Correlator(const WH_Correlator&);
    /// forbid assignment
    WH_Correlator& operator= (const WH_Correlator&);
    

    int itsNsamples;
    int itsNelements;
    int itsNpolarisations;
    int itsNchannels;

  };

} // namespace LOFAR

#endif
