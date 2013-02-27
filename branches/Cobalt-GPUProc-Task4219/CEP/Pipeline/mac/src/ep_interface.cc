#include <lofar_config.h>
#include <boost/python.hpp>
#include "ep_interface.h"

EP_Interface::EP_Interface(std::string servicename, 
                           short protocol_id, 
                           std::string host) : 
  my_EventPort(new LOFAR::MACIO::EventPort(servicename, false, 
                                           protocol_id, host, true))
{
}

EP_Interface::~EP_Interface()
{
  delete my_EventPort;
}

GenericEventWrapper* EP_Interface::receive_event() 
{
  LOFAR::MACIO::GCFEvent* ackPtr;
  Py_BEGIN_ALLOW_THREADS
    ackPtr = my_EventPort->receive();
  Py_END_ALLOW_THREADS
    return new GenericEventWrapper(ackPtr);
}
    
void EP_Interface::send_event(GenericEventWrapper* wrapped_event)
{
  my_EventPort->send(wrapped_event->get_event_ptr());
}

BOOST_PYTHON_MODULE(_ep_interface)
{
  using namespace boost::python;

  // We export GenericEventWrapper here, but it is required by all the
  // protocols. Hence, ep.interface must always be imported first.
  class_<GenericEventWrapper>("GenericEvent")
    .add_property("signal", &GenericEventWrapper::get_signal)
    ;

  class_<EP_Interface>("EP_Interface", 
                       "EP_Interface(ServiceMask, Protocol, Host=localhost)", 
                       init<std::string, short, optional<std::string> >())
    .def("receive_event", &EP_Interface::receive_event, 
         return_value_policy<manage_new_object>())
    .def("send_event", &EP_Interface::send_event)
    ;
}
