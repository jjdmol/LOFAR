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


def create_test_msg(payload):
    """
    Creates a minimal valid msg with payload
    """
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
    msg.payload = payload
    return msg

def get_command_queue_bus(masterCommandQueueName, broker):
    """
    Creates a command queue to bus
    """
    commandQueueBus = msgbus.ToBus(
                   masterCommandQueueName,
              options = "create:never, node: { type: queue, durable: True}",
              broker = broker)

    return commandQueueBus

def get_slave_command_bus(slaveCommandQueueName, broker):
    """
    Helper function, creates validated frombus connected on the expected
    slave bus name
    """

    slaveCommandQueueBus = None
    try:
        slaveCommandQueueBus = msgbus.FromBus(slaveCommandQueueName,
                                              broker = broker)

    except Exception, ex:
        logger.error("Exception thrown by FromBus, this is probably caused"
                     " by the msgbus routing not been set up correctly.")
        raise ex

    return slaveCommandQueueBus


def test_silent_eating_of_incorrect_commands():
    """
    The Command queue daemon should always continue working, even in the case 
    of unknown of incorrect commands
    Send a number of broken msg to the command queue
    """
    # Some settings
    broker =  "locus102"
    busname = "testing9"
    #masterCommandQueueName = busname + "/" + "masterCommandQueueName"
    masterCommandQueueName = "masterCommandQueueName"
    # Create the sut
    daemon = MCQDaemon.MCQDaemon(broker, busname, masterCommandQueueName, 1,
                                 False)

    # connect to the bus
    commandQueueBus =get_command_queue_bus(masterCommandQueueName, broker)


    # Excercise the SUT 

    # Test 1: incorrect command
    payload = {'command':'incorrect',  
                   'node':'locus102',
                   'job':{}}
    msg = create_test_msg(payload)
    commandQueueBus.send(msg)
    daemon._process_commands() # Start the processing on the sut
    
    
    # test 2: No payload
    msg.payload = None
    commandQueueBus.send(msg)
    daemon._process_commands()

    # Cleanup sut
    commandQueueBus.close()



def test_forwarding_of_job_msg_to_queue():
    """

    """
    # config
    broker =  "locus102"
    job_node = 'locus102'
    busname = "testing9"
    #busname = "testbus"
    masterCommandQueueName = busname + "/" + "masterCommandQueueName"
    #masterCommandQueueName = "masterCommandQueueName"
    slaveCommandQueueName = busname + "/" + job_node 

    # create the sut
    daemon = MCQDaemon.MCQDaemon(broker, busname, masterCommandQueueName, 1,
                                 False)

    # connect to the queueus
    commandQueueBus =get_command_queue_bus(masterCommandQueueName, broker)
    slaveCommandQueueBus = get_slave_command_bus(slaveCommandQueueName,
                                                 broker)


    # Test1: Create a test job payuoad
    send_payload =  {'command':'run_job',
                   'node':job_node,
                   'job':{}}

    time.sleep(1)
    msg = create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # start the daemon processing
    daemon._process_commands()
  

    # validate that a job is received on the slave queue

    # wait on the slave command queue
    idx = 0
    msg_received = None

    while (True):
        print "receiving on: {0}".format(slaveCommandQueueName)
        if idx >= 10:
            raise Exception("Did not receive a job command after 10 seconds!!")

        msg_received = slaveCommandQueueBus.get(1)
        if msg_received is None:
            print "Did not receive a msg on the slave command queue"
            idx += 1
            time.sleep(1)
            continue

        slaveCommandQueueBus.ack(msg_received)
        break
    # unpack received data
    received_payload = eval(msg_received.content().payload)

    # validate correct content
    if received_payload != send_payload:
        raise Exception("Send data not the same as received data")

    # Cleanup sut
    commandQueueBus.close()
    slaveCommandQueueBus.close()





if __name__ == "__main__":
    test_silent_eating_of_incorrect_commands()
    test_forwarding_of_job_msg_to_queue()



       

