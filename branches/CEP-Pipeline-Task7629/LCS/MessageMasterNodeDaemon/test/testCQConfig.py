import lofar.messagebus.CQConfig as CQConfig
import lofar.messagebus.msgbus as msgbus
import time

def test_create_queue():
    result_value = None
    try:
        result_value = CQConfig.create_queue_on_node(
            'locus102', 'create_queue_on_node')
    except:
        print "we received an exception"

    print result_value





if __name__ == "__main__":   



    #qpid_route = imp.load_source('', "/opt/qpid/bin/qpid-route")
    #qpid_config = imp.load_source('', "/opt/qpid/bin/qpid-config")


    #make_queue_args = ['-b', 'locus098', 'add', 'queue', 'created_queue_from_locus102']

    #qpid_config.main(argv=make_queue_args)
    ## fed temp_forwarded_queue locus102 locus098
    queueName = "test2"
    print "create 1"
    print CQConfig.create_queue_on_node('locus102',queueName)
    print "create 2"
    print CQConfig.create_queue_on_node('locus098',queueName)
    print "create link"
    try:
      print CQConfig.create_queue_forward('locus102','locus098',queueName)
    except:
      pass
    print "start sleep"
    print "delete link"
    try:
      print CQConfig.delete_queue_forward('locus102','locus098',queueName)
    except SystemExit, e:
      print "we are printing sys exception"
      print e
      pass
    print "delete 1"
    print CQConfig.delete_queue_on_node('locus102',queueName)
    print "delete 2"
    print CQConfig.delete_queue_on_node('locus098',queueName)



    #print "BWWAAAAA"

     #a_bus = msgbus.ToBus('Queue_to_forward', 
     #         options = "create:always, node: { type: queue, durable: False}",
     #         broker = CQConfig.broker )  


     #msg = CQConfig.create_run_job_msg({'data':'data'}, 'locus102', 'locus098')

     #a_bus.send(msg)
