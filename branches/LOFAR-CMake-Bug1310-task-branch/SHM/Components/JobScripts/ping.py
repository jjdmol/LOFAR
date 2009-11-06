#! /usr/bin/env python

import optparse, re
import lofar.shm.db, time

from misconnection       import MISConnection
from lofar.shm.nanostamp import ns2dt

def test_ping(mis):
    for i in range(100):
        (pingtime_ns, echotime_ns, received_ns) = mis.do_ping()
        print "ping_time .............. : <%s>" % ns2dt(pingtime_ns)
        print "peer_echo_time ......... : <%s>" % ns2dt(echotime_ns)
        print "received_echo_time ..... : <%s>" % ns2dt(received_ns)
        print "diff ................... : <%d>" % (received_ns - pingtime_ns)

def test_id(mis):
    mis.get_peer_info()

def test_dp(mis):
    # do dp
    mis.do_dp("PIC_Rack0_SubRack0_Board0_AP0_RCU0.status")

def main():

    parser = optparse.OptionParser(usage="%prog [options] host\n")
    parser.add_option("-p", "--port", dest="mis_port", type='int', default=23995, help="port to query on MIS host (default 23995)")
    parser.add_option("-t", "--type", dest="type", default='ping', help="type of communication ('ping, id')")
    (options, args) = parser.parse_args()

    print "This is MISclient, the MAC Information Server client, v1.1"

    # lofar28

    #    HOST = '10.87.2.128' # The host providing the service
    #    HOST = '10.87.2.181' # The host providing the service
    #    HOST = '10.127.127.1' # cs001t
    #    HOST = '127.0.0.1' 
    #    HOST = '10.87.2.183'  # The host providing the service
    #    HOST = '10.151.18.1'  # The host providing the service
    #    HOST = '10.151.0.1'  # The host providing the service
    #    PORT =   27009        # The service port
    #    PORT =   23990        # The service port
    #    PORT =   24005        # The service port
    #    PORT =   9001        # The service port

    HOST = args[0]                   # The host providing the MACInformationServer (MIS)
    if (options.mis_port is not None):   
        PORT = options.mis_port      # The MIS TCP port
    
    mis = MISConnection(HOST, PORT)
    print "connecting..."
    mis.connect()
    
    if re.match('ping',options.type):
        print "going to ping"
        test_ping(mis)
        print "have pinged"

    elif re.match('id',options.type):
        print "going to id"
        test_id(mis)
        print "have id-ed"

    elif re.match('dp',options.type):
        print "going to dp"
        test_dp(mis)
        mis.do_dp("PIC_WAN_BackBone_FieldCN.port3.TxT", mode = "SUBSCRIBE")
        mis.do_dp("MIS)S.TxT", mode = "SUBSCRIBE")
        print "have dp-ed"
    
    # disconnect
    print "closing..."
    mis.disconnect()

    print "Thank you for using MISclient."

# main program

if __name__ == "__main__":
    main()
