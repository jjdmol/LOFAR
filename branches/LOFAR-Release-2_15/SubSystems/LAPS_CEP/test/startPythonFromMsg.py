#!/usr/bin/python
import sys
from LAPS.MsgBus.Bus import Bus

# Create queue with a unique name

# insert message

# receive msg

# delete queue


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    try:
        unique_queue_name = sys.argv[1]
    except:
        print "Not enough command line arguments: this test needs a unique queue name"
        exit(1)


    #msgbus = Bus(broker="lhd002", address=unique_queue_name)

    #parset = """
#key=value
#"""

    #msgbus.send(parset,"Observation123456")