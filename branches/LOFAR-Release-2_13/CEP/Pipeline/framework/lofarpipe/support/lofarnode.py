#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                            Compute node system
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os
import socket
import random
import time
import struct
import platform
import logging
import logging.handlers
import cPickle as pickle

from lofarpipe.support.usagestats import UsageStats

def run_node(*args):
    """
    Run on node to automatically locate, instantiate and execute the
    correct LOFARnode class.
    """
    import imp
    control_script = getattr(
        imp.load_module(recipename, *imp.find_module(recipename, [nodepath])),
        recipename
    )
    return control_script(loghost=loghost, logport=logport).run_with_logging(*args)

class LOFARnode(object):
    """
    Base class for node jobs called through IPython or directly via SSH.

    Sets up TCP based logging.
    """
    def __init__(
        self,
        loghost=None,
        logport=logging.handlers.DEFAULT_TCP_LOGGING_PORT
    ):
        self.logger = logging.getLogger(
            'node.%s.%s' % (platform.node(), self.__class__.__name__)
        )
        self.logger.setLevel(logging.DEBUG)
        self.loghost = loghost
        self.logport = int(logport)
        self.outputs = {}
        self.environment = os.environ
        self.resourceMonitor = UsageStats(self.logger, 10.0 * 60.0)  # collect stats each 10 minutes      

    def run_with_logging(self, *args):
        """
        Calls the run() method, ensuring that the logging handler is added
        and removed properly.
        """
        if self.loghost:
            my_tcp_handler = logging.handlers.SocketHandler(self.loghost, self.logport)
            self.logger.addHandler(my_tcp_handler)
        try:
            self.resourceMonitor.start()
            return_value = self.run(*args)
            self.outputs["monitor_stats"] = \
                    self.resourceMonitor.getStatsAsXmlString()
            return return_value
        finally:
            self.resourceMonitor.setStopFlag()
            if self.loghost:
                my_tcp_handler.close()
                self.logger.removeHandler(my_tcp_handler)

    def run(self):
        # Override in subclass.
        raise NotImplementedError

class LOFARnodeTCP(LOFARnode):
    """
    This node script extends :class:`~lofarpipe.support.lofarnode.LOFARnode`
    to receive instructions via TCP from a
    :class:`~lofarpipe.support.jobserver.JobSocketReceiver`.
    """
    def __init__(self, job_id, host, port):
        self.job_id, self.host, self.port = int(job_id), host, int(port)
        self.__fetch_arguments()
        super(LOFARnodeTCP, self).__init__(self.host, self.port)

    def run_with_stored_arguments(self):
        """
        After fetching arguments remotely, use them to run the standard
        run_with_logging() method.
        """
        returnvalue = self.run_with_logging(*self.arguments)
        self.__send_results()
        return returnvalue

    def __try_connect(self, sock, tries=5, min_timeout=1.0, max_timeout=5.0):
        """
        Try to connect to the remote job dispatch server, using the socket
        `sock`. For reasons not yet understood, the socket.connect() method
        often times out on the CEP-II cluster. Try to connect up to
        `tries` times, using a random time-out interval ([`min_timeout` ..
        `max_timeout`) seconds) between retries.
        """
        while True:
            tries -= 1
            try:
                sock.connect((self.host, self.port))
            except socket.error, e:
                print("Could not connect to %s:%s (got %s)" %
                      (self.host, str(self.port), str(e)))
                if tries > 0:
                    timeout = random.uniform(min_timeout, max_timeout)
                    print("Retrying in %f seconds (%d more %s)." %
                          (timeout, tries, "try" if tries == 1 else "tries"))
                    time.sleep(timeout)
                else:
                    raise
            else:
                break

    def __fetch_arguments(self, tries=5, min_timeout=1.0, max_timeout=5.0):
        """
        Connect to a remote job dispatch server (an instance of
        jobserver.JobSocketReceive) and obtain all the details necessary to
        run this job.
        """
        while True:
            tries -= 1
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.__try_connect(s)
                message = "GET %d" % self.job_id
                s.sendall(struct.pack(">L", len(message)) + message)
                chunk = s.recv(4)
                slen = struct.unpack(">L", chunk)[0]
                chunk = s.recv(slen)
                while len(chunk) < slen:
                    chunk += s.recv(slen - len(chunk))
                self.arguments = pickle.loads(chunk)
            except socket.error, e:
                print "Failed to get recipe arguments from server"
                if tries > 0:
                    timeout = random.uniform(min_timeout, max_timeout)
                    print("Retrying in %f seconds (%d more %s)." %
                          (timeout, tries, "try" if tries == 1 else "tries"))
                    time.sleep(timeout)
                else:
                    # we tried 5 times, abort with original exception
                    raise 
            else:
                # no error, thus break the loop
                break  #

    def __send_results(self):
        """
        Send the contents of self.outputs to the originating job dispatch
        server.
        """
        message = "PUT %d %s" % (self.job_id, pickle.dumps(self.outputs))

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__try_connect(s)
        s.sendall(struct.pack(">L", len(message)) + message)
