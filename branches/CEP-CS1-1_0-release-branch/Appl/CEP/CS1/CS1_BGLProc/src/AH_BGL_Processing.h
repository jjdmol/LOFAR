//#  AH_BGL_Processing.h: 
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

#ifndef LOFAR_CS1_BGL_PROC_AH_BGL_PROCESSING_H
#define LOFAR_CS1_BGL_PROC_AH_BGL_PROCESSING_H

#include <tinyCEP/TinyApplicationHolder.h>
#include <CS1_Interface/Stub_BGL.h>
#include <CS1_Interface/CS1_Parset.h>

#if defined HAVE_BGL
#include <rts.h>
#endif

namespace LOFAR {
namespace CS1 {

    //# Forward declarations
class WH_BGL_Processing;

    // Description of class.
class AH_BGL_Processing : public TinyApplicationHolder
{
  public:
		 AH_BGL_Processing();
    virtual	 ~AH_BGL_Processing();
    virtual void undefine();
    virtual void define(const KeyValueMap&);
    virtual void init();
    virtual void run(int nsteps);
    /*   virtual void postrun  (); */
    virtual void dump() const;
    virtual void quit();

    static char  **original_argv;

  private:
#if defined HAVE_BGL
    static unsigned remapOnTree(unsigned cell, unsigned core, struct BGLPersonality &);
#endif
    
    CS1_Parset                  *itsCS1PS;
    vector<WH_BGL_Processing *> itsWHs;
    Stub_BGL			*itsSubbandStub, *itsVisibilitiesStub;
};

} // namespace CS1
} // namespace LOFAR

#endif
