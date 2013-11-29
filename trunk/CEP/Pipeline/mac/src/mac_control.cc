#include <lofar_config.h>
#include <boost/python.hpp>
#include <APL/APLCommon/Controller_Protocol.ph>

namespace LOFAR
{
  namespace Pipeline
  {
    BOOST_PYTHON_MODULE(_mac_control)
    {
      using namespace boost::python;
      using namespace Controller_Protocol;

      //
      // Protocol ID
      //
      scope().attr("PROTOCOL")          = (short) CONTROLLER_PROTOCOL;

      //
      // Controller error types
      //
      scope().attr("NO_ERR")            = (short) CONTROL_NO_ERR;
      scope().attr("LOST_CONN")         = (short) CONTROL_LOST_CONN_ERR;

      //
      // Controller message types
      //
      scope().attr("CONTROL_STARTED")   = CONTROL_STARTED;
      scope().attr("CONTROL_CONNECT")   = CONTROL_CONNECT;
      scope().attr("CONTROL_CONNECTED") = CONTROL_CONNECTED;
      scope().attr("CONTROL_RESYNC")    = CONTROL_RESYNC;
      scope().attr("CONTROL_RESYNCED")  = CONTROL_RESYNCED;
      scope().attr("CONTROL_SCHEDULE")  = CONTROL_SCHEDULE;
      scope().attr("CONTROL_SCHEDULED") = CONTROL_SCHEDULED;
      scope().attr("CONTROL_CLAIM")     = CONTROL_CLAIM;
      scope().attr("CONTROL_CLAIMED")   = CONTROL_CLAIMED;
      scope().attr("CONTROL_PREPARE")   = CONTROL_PREPARE;
      scope().attr("CONTROL_PREPARED")  = CONTROL_PREPARED;
      scope().attr("CONTROL_RESUME")    = CONTROL_RESUME;
      scope().attr("CONTROL_RESUMED")   = CONTROL_RESUMED;
      scope().attr("CONTROL_SUSPEND")   = CONTROL_SUSPEND;
      scope().attr("CONTROL_SUSPENDED") = CONTROL_SUSPENDED;
      scope().attr("CONTROL_RELEASE")   = CONTROL_RELEASE;
      scope().attr("CONTROL_RELEASED")  = CONTROL_RELEASED;
      scope().attr("CONTROL_QUIT")      = CONTROL_QUIT;
      scope().attr("CONTROL_QUITED")    = CONTROL_QUITED;

      //
      // Protocol event classes
      //
      class_<MACIO::GCFEvent, boost::noncopyable>("GCFEvent", no_init);
//         .def_readonly("signal");

      class_<CONTROLStartedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLStartedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLConnectEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLConnectEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLConnectedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLConnectedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLResyncEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLResyncEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLResyncedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLResyncedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLScheduleEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLScheduleEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLScheduledEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLScheduledEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLClaimEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLClaimEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLClaimedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLClaimedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLPrepareEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLPrepareEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLPreparedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLPreparedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLResumeEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLResumeEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLResumedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLResumedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLSuspendEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLSuspendEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLSuspendedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLSuspendedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLReleaseEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLReleaseEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLReleasedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLReleasedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLQuitEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLQuitEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLQuitedEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLQuitedEvent", init<MACIO::GCFEvent&>());

      class_<CONTROLCommonEvent, bases<MACIO::GCFEvent>, boost::noncopyable>
        ("CONTROLCommonEvent", init<MACIO::GCFEvent&>());

    }
  }
}
