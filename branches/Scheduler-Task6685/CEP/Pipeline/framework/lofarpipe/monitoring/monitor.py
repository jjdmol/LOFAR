#! /usr/bin/env python


from config import Config # this is a subdir
from listener import Listener
from poller import Poller
import sys

if __name__ == "__main__":
    """Run the monitor. The listener class waits for requests. The
    poller class polls the PIDs that were input and forwards output
    to the output class."""
    parpid = sys.argv[1]
    cfg = Config()
    cfg.add_item('parentpid',parpid)
    lst = Listener(cfg)
    lst.start()
    print "listener started"
    # Where is the output class?
    
    pol = Poller(cfg)   
    pol.start()
    print "Poller started"