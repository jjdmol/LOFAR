//#  WH_ION_Scatter.h: simple processing on BG/L I/O nodes
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_ION_PROC_WH_ION_SCATTER_H
#define LOFAR_APPL_CEP_CS1_CS1_ION_PROC_WH_ION_SCATTER_H

#include <CS1_Interface/DH_Subband.h>
//#include <Transport/TH_ZoidServer.h>
#include <tinyCEP/WorkHolder.h>

#include <vector>


namespace LOFAR {
namespace CS1 {

class WH_ION_Scatter : public WorkHolder
{
  public:
    explicit WH_ION_Scatter(const string &name, const CS1_Parset *ps);
    virtual  ~WH_ION_Scatter();

    //static WorkHolder *construct(const string &name, const ACC::APS::ParameterSet &);
    virtual WH_ION_Scatter *make(const string &name);

    virtual void preprocess();
    virtual void process();
    virtual void postprocess();

  private:
    // forbid copy constructor
    WH_ION_Scatter(const WH_ION_Scatter &);

    // forbid assignment
    WH_ION_Scatter &operator = (const WH_ION_Scatter &);

    unsigned			itsNrComputeNodes, itsCurrentComputeNode;
    //vector<TH_ZoidServer *>	itsOutputs;
    const CS1_Parset		*itsPS;
};

}
}

#endif
