//#  WH_ION_Gather.h: simple processing on BG/L I/O nodes
//#
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_ION_PROC_WH_ION_GATHER_H
#define LOFAR_APPL_CEP_CS1_CS1_ION_PROC_WH_ION_GATHER_H

#include <CS1_Interface/DH_Visibilities.h>
#include <tinyCEP/WorkHolder.h>
#include <Transport/TransportHolder.h>
#include <APS/ParameterSet.h>

#include <vector>


namespace LOFAR {
namespace CS1 {

class WH_ION_Gather : public WorkHolder
{
  public:
	     WH_ION_Gather(const string &name, unsigned psetNumber, const CS1_Parset *ps, const std::vector<TransportHolder *> &clientTHs);
    virtual  ~WH_ION_Gather();

    //static WorkHolder *construct(const string &name, const ACC::APS::ParameterSet &);
    virtual WH_ION_Gather *make(const string &name);

    virtual void preprocess();
    virtual void process();
    virtual void postprocess();

  private:
    // forbid copy constructor
    WH_ION_Gather(const WH_ION_Gather &);

    // forbid assignment
    WH_ION_Gather &operator = (const WH_ION_Gather &);

    vector<DH_Visibilities *>	itsSumDHs;
    DH_Visibilities		*itsTmpDH;

    unsigned			itsPsetNumber, itsNrComputeCores, itsCurrentComputeCore;
    unsigned			itsNrSubbandsPerPset, itsCurrentSubband;
    unsigned			itsNrIntegrationSteps, itsCurrentIntegrationStep;

    const CS1_Parset		*itsPS;
    const std::vector<TransportHolder *> &itsClientTHs;
};

}
}

#endif
