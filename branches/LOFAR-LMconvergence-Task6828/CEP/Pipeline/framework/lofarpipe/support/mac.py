#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                       Pipeline MAC integration
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import threading
import time
import collections

from lofarpipe.support.control import control
from lofarpipe.support.lofarexceptions import PipelineException, PipelineQuit
import lofarpipe.support.lofaringredient as ingredient

#                                            Required by MAC EventPort Interface
# ------------------------------------------------------------------------------

from ep.control import *
from ep.control import OK as controlOK

#                                                           Integration with MAC
# ------------------------------------------------------------------------------

class MAC_control(control):
    """
    This extends the control framework to interface with MAC.
    """
    inputs = {
        'controllername': ingredient.StringField(
            '--controllername',
            help="Controller name"
            ),
        'servicemask': ingredient.StringField(
            '--servicemask',
            help="Service mask"
            ),
        'targethost': ingredient.StringField(
            '--targethost',
            help="Target host"
            ),
        'treeid': ingredient.IntField(
            '--treeid',
            help="Tree ID"
            )
        }

    def __init__(self):
        super(MAC_control, self).__init__()

    def pipeline_logic(self):
        """
        Define pipeline logic in subclasses.
        """
        raise NotImplementedError

    def run_task(self, configblock, datafiles=[]):
        self.logger.info( "Waiting for run state...")
        self.state['run'].wait()
        self.logger.info( "Running.")
        self.logger.debug("Quit is %s" % (str(self.state['quit'].isSet())))
        if self.state['quit'].isSet():
            self.logger.info("Pipeline instructed to quit; bailing out")
            raise PipelineQuit
        try:
            super(MAC_control, self).run_task(configblock, datafiles)
        except PipelineException, message:
            self.logger.warn(message)
#            raise PipelineQuit

    def go(self):
        #     Pipeline logic proceeds as in a standard recipe in its own thread
        #                          MAC control takes place in a separate thread
        # ---------------------------------------------------------------------
        super(MAC_control, self).go()

        self.logger.info(
            "LOFAR Pipeline (%s) starting." %
            (self.name,)
        )

        self.state = {
            'run':      threading.Event(),
            'quit':     threading.Event(),
            'pause':    threading.Event(),
            'finished': threading.Event()
        }

        control_thread = threading.Thread(target=self.control_loop)
        pipeline_thread = threading.Thread(target=self.pipeline_logic)

        pipeline_thread.setDaemon(True)
        control_thread.start()
        pipeline_thread.start()
        control_thread.join()
        self.logger.info("Control loop finished; shutting down")
        return 0

    def control_loop(self):
        """
        Loop until the pipeline finishes, receiving and responding to messages
        sent by MAC.
        """
        #                                             Connect to the MAC server
        # ---------------------------------------------------------------------
        try:
            my_interface = ControllerPort_Interface(
                self.inputs['servicemask'], self.inputs['targethost']
            )
        except:
            self.logger.error("Control interface not connected; quitting")
            self.state['quit'].set()
            self.state['run'].set()
            return
        my_interface.send_event(
            ControlConnectEvent(self.inputs['controllername'])
        )

        #                    Buffer events received from the EventPort interface
        # ----------------------------------------------------------------------
        class ReceiverThread(threading.Thread):
            def __init__(self, interface, logger):
                super(ReceiverThread, self).__init__()
                self.interface = interface
                self.logger = logger
                self.event_queue = collections.deque()
                self.active = True
            def run(self):
                while self.active:
                    self.event_queue.append(self.interface.receive_event())
                    self.logger.debug("Got a new event")
            def next_event(self):
                try:
                    return self.event_queue.popleft()
                except IndexError:
                    return None
        event_receiver = ReceiverThread(my_interface, self.logger)
        event_receiver.setDaemon(True)
        event_receiver.start()
        controllername = self.inputs['controllername']

        #            The main control loop continues until the pipeline finshes
        # ---------------------------------------------------------------------
        while True:
            #                               Handle any events received from MAC
            # -----------------------------------------------------------------
            current_event = event_receiver.next_event()

            if isinstance(current_event, ControlConnectedEvent):
                self.logger.debug("Received ConnectedEvent")
            elif isinstance(current_event, ControlClaimEvent):
                self.logger.debug("Received ClaimEvent")
                my_interface.send_event(
                    ControlClaimedEvent(controllername, controlOK)
                )
            elif isinstance(current_event, ControlPrepareEvent):
                self.logger.debug("Received PrepareEvent")
                my_interface.send_event(
                    ControlPreparedEvent(controllername, controlOK)
                )
            elif isinstance(current_event, ControlSuspendEvent):
                self.logger.debug("Received SuspendEvent")
                self.logger.debug("Clearing run state; pipeline must pause")
                self.state['run'].clear()
                my_interface.send_event(
                    ControlSuspendedEvent(controllername, controlOK)
                )
            elif isinstance(current_event, ControlResumeEvent):
                self.logger.debug("Received ResumeEvent")
                self.logger.debug("Setting run state: pipeline may run")
                self.state['run'].set()
                my_interface.send_event(
                    ControlResumedEvent(controllername, controlOK)
                )
            elif isinstance(current_event, ControlReleaseEvent):
                self.logger.debug("Received ReleaseEvent")
                my_interface.send_event(
                    ControlReleasedEvent(controllername, controlOK)
                )
            elif isinstance(current_event, ControlQuitEvent):
                self.logger.debug("Received QuitEvent")
                self.logger.debug("Setting quit state: pipeline must exit")
                self.state['quit'].set()
                self.state['run'].clear()
                my_interface.send_event(
                    ControlQuitedEvent(
                        controllername,
                        self.inputs['treeid'],
                        controlOK,
                        "no error"
                    )
                )
            elif isinstance(current_event, ControlResyncEvent):
                self.logger.debug("Received ResyncEvent")
                my_interface.send_event(
                    ControlResyncedEvent(controllername, controlOK)
                )
            elif isinstance(current_event, ControlScheduleEvent):
                self.logger.debug("Received ScheduleEvent")
                my_interface.send_event(
                    ControlScheduledEvent(controllername, controlOK)
                )

            #                  Shut everything down if the pipeline is finished
            # -----------------------------------------------------------------
            if self.state['finished'].isSet():
                self.logger.debug("Got finished state: control loop exiting")
                my_interface.send_event(
                    ControlQuitedEvent(
                        controllername,
                        self.inputs['treeid'],
                        controlOK,
                        "pipeline finished"
                    )
                )
                event_receiver.active = False
                break

            self.logger.debug("Control looping...")
            time.sleep(1)


#                                                                      Self test
# ------------------------------------------------------------------------------
if __name__ == "__main__":
    
    class HelloWorldPipeline(MAC_control):
        def __init__(self):
            super(HelloWorldPipeline, self).__init__()
            job_name = "hello"
            
        def pipeline_logic(self):
            print "Hello World"
        
    import sys
    sys.exit(HelloWorldPipeline().main())
