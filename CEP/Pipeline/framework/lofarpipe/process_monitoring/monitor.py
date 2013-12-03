#! /usr/bin/env python
from config import Config
from listener import Listener
from poller import Poller
import sys

if __name__ == "__main__":
    """Run the monitor. The listener class waits for requests. The
    poller class polls the PIDs that where input and forwards output
    to the output class."""
    parpid = sys.argv[1]
    cfg = Config()
    cfg.add_item('parentpid',parpid)
    lst = Listener(cfg)
    lst.start()
    
    pol = Poller(cfg)
    pol.start()