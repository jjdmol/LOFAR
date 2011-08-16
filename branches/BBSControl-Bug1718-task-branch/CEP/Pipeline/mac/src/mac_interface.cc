#include <lofar_config.h>
#include <string>
#include <MACIO/EventPort.h>
#include <boost/python.hpp>

namespace LOFAR
{
  namespace Pipeline
  {
    using namespace std;

    class MACInterface : boost::noncopyable
    {
    public:
      MACInterface(const string& servicename, 
                   short protocol_id,
                   const string& host = "");
      MACIO::GCFEvent* receiveEvent();
      void sendEvent(MACIO::GCFEvent* event);
    private:
      MACIO::EventPort itsEventPort;
    };

    MACInterface::MACInterface(const string& servicename, 
                               short protocol_id,
                               const string& host) :
      itsEventPort(servicename, false, protocol_id, host, true)
    {
    }

    MACIO::GCFEvent* MACInterface::receiveEvent()
    {
      MACIO::GCFEvent* event(0);
      Py_BEGIN_ALLOW_THREADS
        event = itsEventPort.receive();
      Py_END_ALLOW_THREADS
      return event;
    }

    void MACInterface::sendEvent(MACIO::GCFEvent* event)
    {
      Py_BEGIN_ALLOW_THREADS
        itsEventPort.send(event);
      Py_END_ALLOW_THREADS
    }

    BOOST_PYTHON_MODULE(_mac_interface)
    {
      using namespace boost::python;
      class_<MACInterface, boost::noncopyable>
        ("MACInterface", init<const string&, short, const string&>())
        .def("receive_event", &MACInterface::receiveEvent,
             return_value_policy<manage_new_object>())
        .def("send_event", &MACInterface::sendEvent);
    }

  }
}
