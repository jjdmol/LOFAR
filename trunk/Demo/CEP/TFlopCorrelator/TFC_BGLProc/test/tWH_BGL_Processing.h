//#  filename.h: one line description
//#
//#  Copyright (C) 2002-2005
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

#ifndef TFC_TWH_BGL_PROCESSING_H
#define TFC_TWH_BGL_PROCESSING_H

#include <tinyCEP/TinyApplicationHolder.h>
#include <tinyCEP/WorkHolder.h>
#include <Blob/KeyValueMap.h>
#include <Transport/Connection.h>
#include <Transport/TransportHolder.h>
#include <Transport/DataHolder.h>

/* #define NR_CORRELATORS_PER_FILTER	5  */


namespace LOFAR {

  class AH_BGL_Processing : public LOFAR::TinyApplicationHolder {

  public:
    AH_BGL_Processing ();
    virtual ~AH_BGL_Processing ();

    virtual void define (const KeyValueMap& kvm);
    void undefine();
    virtual void init();
    virtual void run(int nsteps);
    virtual void dump() const;
    virtual void postrun();
    virtual void quit();

  private:

    WorkHolder	    *itsWH;
    TransportHolder *itsTH;
    DataHolder	    *itsInDH,   *itsOutDH;
    Connection	    *itsInConn, *itsOutConn;
  };

} 
#endif
