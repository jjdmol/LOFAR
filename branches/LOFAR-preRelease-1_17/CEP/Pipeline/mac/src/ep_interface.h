#ifndef EP_INTERFACE_H
#define EP_INTERFACE_H

/*!
  \file ep_interface.h
  \ingroup pipeline
*/

#include <MACIO/EventPort.h>
#include <string>

#include "GenericEventWrapper.h"

/*!
  \class EP_Interface
  \ingroup pipeline
  \brief Event Port Interface
*/
class EP_Interface
{
public:
  EP_Interface(std::string servicename, 
               short protocol_id, 
               std::string host = "");
  GenericEventWrapper* receive_event();
  void send_event(GenericEventWrapper* wrapped_event);
  ~EP_Interface();
private:
  LOFAR::MACIO::EventPort* my_EventPort;
};

#endif
