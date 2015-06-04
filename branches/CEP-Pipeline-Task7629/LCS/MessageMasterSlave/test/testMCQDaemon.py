# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

import logging
import time

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import lofar.messagebus.MCQDaemon as MCQDaemon


# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("MessageBus")



def test_silent_eating_of_unknown_commands():
    """
    The Command queue daemon should always continue working, even in the case 
    of unknown of incorrect commands
    Send a number of broken msg to the command queue
    """
    broker =  "locus102"
    busname = "testbus"
    masterCommandQueueName = "masterCommandQueueName"
    daemon = MCQDaemon.MCQDaemon(broker, busname, masterCommandQueueName, 1,
                                 False)

    # create a connection to the command queue

    # Connect to the command queue
    commandQueueBus = msgbus.ToBus(
                   masterCommandQueueName,
              options = "create:never, node: { type: queue, durable: True}",
              broker = broker)


    msg = message.MessageContent(
                from_="test",
                forUser="MCQDaemon",
                summary="summary",
                protocol="protocol",
                protocolVersion="test", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )

    # Unknown command
    msg.payload = {'command':'incorrect',
                   'node':'locus12',
                   'job':{}}
    commandQueueBus.send(msg)
    daemon._process_commands()

    # No payload
    msg.payload = None
    commandQueueBus.send(msg)
    daemon._process_commands()



def test_forwarding_of_job_msg_to_queue():
    """

    """
    broker =  "locus102"
    busname = "testbus10"
    masterCommandQueueName = "masterCommandQueueName2"
    daemon = MCQDaemon.MCQDaemon(broker, busname, masterCommandQueueName, 1,
                                 False)

    job_node = 'locus102'
    # create a connection to the command queue

    # Connect to the command queue
    commandQueueBus = msgbus.ToBus(
                   masterCommandQueueName,
              options = "create:never, node: { type: queue, durable: True}",
              broker = broker)

    slaveCommandQueueName = busname + "/" + job_node 
    # connect to the receive bus for the node
    slaveCommandQueueBus = None
    try:
        print 
        slaveCommandQueueBus = msgbus.FromBus(
                  slaveCommandQueueName,
              broker = broker)

    except Exception, ex:
        logger.error("Exception thrown by FromBus, this is probably caused"
                     " by the msgbus routing not been set up correctly.")
        raise ex

    msg = message.MessageContent(
                from_="test",
                forUser="MCQDaemon",
                summary="summary",
                protocol="protocol",
                protocolVersion="test", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )

    send_payload =  {'command':'run_job',
                   'node':job_node,
                   'job':{}}
    msg.payload = send_payload
    print "sending msg on {0}".format(masterCommandQueueName)
    commandQueueBus.send(msg)

    daemon._process_commands()
    msg_received = None
    while (True):
        msg_received = slaveCommandQueueBus.get(1)
        if msg_received is None:
            print "get none from queue"
            time.sleep(1)
            continue


        print "we received a msg"
        slaveCommandQueueBus.ack(msg_received)
        break

    print "***********************"
    print msg_received.content()
    print "***********************"


    received_payload = eval(msg_received.content().payload)
    print received_payload

    if received_payload != send_payload:
        raise Exception("Send data not the same as received data")




if __name__ == "__main__":
    #test_silent_eating_of_unknown_commands()
    test_forwarding_of_job_msg_to_queue()



       

