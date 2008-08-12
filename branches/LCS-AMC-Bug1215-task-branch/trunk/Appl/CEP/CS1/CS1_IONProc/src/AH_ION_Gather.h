//#  AH_ION_Gather.h: 
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

#ifndef LOFAR_CS1_ION_PROC_AH_ION_GATHER_H
#define LOFAR_CS1_ION_PROC_AH_ION_GATHER_H

#include <CEPFrame/ApplicationHolder.h>
#include <CS1_Interface/Stub_BGL.h>
#include <Stream/Stream.h>
#include <WH_ION_Gather.h>


namespace LOFAR {
namespace CS1 {

class AH_ION_Gather : public ApplicationHolder
{
  public:
		 AH_ION_Gather(const std::vector<Stream *> &clientTHs, unsigned myPsetNumber);
    virtual	 ~AH_ION_Gather();
    virtual void undefine();
    virtual void define(const KeyValueMap&);
    virtual void prerun();
    virtual void run(int nsteps);
    virtual void postrun();

  private:
    CS1_Parset	  *itsCS1PS;
    WH_ION_Gather *itsWH;
    Stub_BGL	  *itsVisibilitiesStub;
    unsigned	  itsPsetNumber;

    const std::vector<Stream *> &itsClientStreams;
};

} // namespace CS1
} // namespace LOFAR

#endif
