//#  -*- mode: c++ -*-
//#
//#  rspctlfe.h: Front end for the command line interface to the RSPDriver
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

#ifndef RSPCTLFE_H_
#define RSPCTLFE_H_

#include <string>
#include <boost/shared_ptr.hpp>

#include <GCF/TM/GCF_Control.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/PAL/GCF_MyPropertySet.h>

namespace LOFAR
{
  
namespace rspctl
{

class RSPCtlFE;

/**
 * Answer class for propertysets
 */
class RSPCtlFEAnswer : public GCF::PAL::GCFAnswer
{
  public:
    RSPCtlFEAnswer(RSPCtlFE& rspctlfe);
    void handleAnswer(GCF::TM::GCFEvent& answer);
    
  private:
    RSPCtlFE& _rspctlfe;
};


/**
 * Controller class for rspctlfe
 */
class RSPCtlFE : public GCFTask
{
public:
  static const string FE_PROPSET_NAME;
  static const string FE_PROPSET_TYPENAME;
  static const string FE_PROPERTY_COMMAND;
  static const string FE_PROPERTY_STATUS;
  
  /**
   * The constructor of the RSPCtlFE task.
   * @param name The name of the task. The name is used for looking
   * up connection establishment information using the GTMNameService and
   * GTMTopologyService classes.
   */
  RSPCtlFE(string name, int argc, char** argv);
  virtual ~RSPCtlFE();

  // state methods

  /**
   * The initial state. In this state the FE server is started
   */
  GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

  /**
   * In this state the Front End waits for updates of the command property
   * and writes status changes of the rspctl to the status property
   */
  GCFEvent::TResult operational(GCFEvent& e, GCFPortInterface &p);

  /**
   * Start the controller main loop.
   */
  void mainloop();

  void valueChanged(const string& propName, const string& value);
  
private:
  // private methods
  void parse_options(int argc, char** argv);
  void pollRspCtlOutput();

private:
  typedef boost::shared_ptr< GCF::PAL::GCFMyPropertySet > GCFMyPropertySetSharedPtr;

  // property sets
  RSPCtlFEAnswer              m_propertySetAnswer;
  int                         m_id;
  GCFMyPropertySetSharedPtr   m_myPropertySet;
  // server port
  GCF::TM::GCFTCPPort         m_server;
  // rspctl output  
  long                        m_rspctlOutputTimer;
  long                        m_rspctlOutputPosition;
  string                      m_rspctlOutputFilename;

  ALLOC_TRACER_CONTEXT  
};

};

};

#endif /* RSPCTLFE_H_ */
