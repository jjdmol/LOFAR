//#  SyncAction.h: RSP Driver syncaction class
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

#ifndef SYNCACTION_H_
#define SYNCACTION_H_

#include <GCF/GCF_Control.h>

namespace RSP
{
  class SyncAction : public GCFFsm
      {
      public:
	  /**
	   * Constructors for a SyncAction object.
	   */
	  explicit SyncAction(State initial);
	  
	  /* Destructor for SyncAction. */
	  virtual ~SyncAction();

	  /**
	   * Set the priority. The priority determines when
	   * a synaction is carried out relative to other
	   * sync actions.
	   */
	  void setPriority(int priority);

	  /*@{*/
	  /**
	   * Has the state machine reached its final state?
	   */
	  void setFinal(bool final);
	  bool isFinal() const;
	  /*@}*/

      private:
	  SyncAction();

      private:
	  int  m_priority;
	  bool m_final; /** indicates whether the state machine has reached its final state */
      };
};
     
#endif /* SYNCACTION_H_ */
