#!/usr/bin/python

import os
import sys
import time
import logging
import lofar.shm
import optparse
import misconnection
import socket
#import xcstats_classifier_max
import rcu_settings_masks
import xcstats_classifier_max_v2
import numpy, string
import array
import re

import pickle

from lofar.shm.nanostamp import ns2dt, now2ns

EXIT_ERROR = 127

def main():

    parser = optparse.OptionParser(usage="%prog [options] host\n")
    parser.add_option("-j", "--job-control", action="store_true", dest="jobctrl", default=False, help="run this program under LOFAR job control")
    parser.add_option("-c", "--commit", action="store_true", dest="commit", default=False, help="commit XST data to SHM database (default: false)")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="write debug information to syslog (default: false)")
    parser.add_option("-p", "--port", dest="mis_port", type='int', help="port to query on MIS host (default: from SHM database)")
    parser.add_option("-s", "--subband", type="int", dest="subband", default=0, help="Subband selection for Station ACM matrix (default: 0)")
    (options, args) = parser.parse_args()

    print "This is the LOFAR-SHM Antenna Correlation Matrix Fetcher Daemon, v1.1"
    
    if len(args) != 1:
        parser.print_help()
        sys.exit(EXIT_ERROR)

    if (options.subband <0) or (options.subband > 511):
        parser.error("Subband " + str(options.subband) + " out of range [0:511]")
        
    # open connection to shm database

    db = lofar.shm.db.SysHealthDatabase()
    db.open()

    if options.verbose:
        log_level = logging.DEBUG
    else:
        log_level = logging.INFO
        
    if options.jobctrl:
        # initialize job control
        jobctrl = lofar.shm.job_control.JobControl(db)

        # get job identifier
        job = lofar.shm.job_control.Job(os.getenv("JOB_ID"), os.getenv("JOB_TICKET_NO"))

        logging.getLogger("").setLevel(log_level)
        handler = lofar.shm.job_control.JobLogHandler(db, job)
        handler.setFormatter(logging.Formatter("%(levelname)s: %(message)s"))
        logging.getLogger("").addHandler(handler)
    else:
        logging.basicConfig(level=log_level,
                            datefmt='%Y-%m-%d %H:%M:%S',
                            format='%(asctime)s.%(msecs)03d %(levelname)-10s | %(message)s')

    # host information from SHM db
    if (args[0].find(".") != -1):
        # inet provided
        MIS_HOST = args[0]                   # The host providing the MACInformationServer (MIS)
        MIS_SI_ID = 'UNKNOWN'
        if (options.mis_port is not None):   
            MIS_PORT = options.mis_port      # The MIS TCP port
        else:
            logging.error(" Must supply MIS port with --port for INET %s "%(MIS_HOST))
            sys.exit(EXIT_ERROR)
    else:
        # from db
        query = "SELECT mis_address, mis_port, si_name, si_id from lofar.macinformationservers WHERE (si_name = '%s');" % (args[0].upper())
        try:
            mistab = db.perform_query(query)
            if (len(mistab) != 1):
                logging.error(" %s not a recognized MIS host name; give INET addr and MIS port instead"%(args[0]))
                db.close()
                sys.exit(EXIT_ERROR)
            else:
                MIS_HOST = mistab[0].mis_address
                MIS_SI_ID = mistab[0].si_id
                if (options.mis_port is not None): 
                    MIS_PORT = options.mis_port
                else:
                    MIS_PORT = mistab[0].mis_port          
        except lofar.shm.DatabaseError, ex:
            logging.error("could not query data: %s (%s)" % (query, ex))
            db.close()
            sys.exit(EXIT_ERROR)

    logging.info("Connecting to MIS on %(MIS_HOST)s on port %(MIS_PORT)s..." % vars())
    mis = misconnection.MISConnection(MIS_HOST, MIS_PORT)
    #has initialized 
    
    try:
        mis.connect()
    except socket.error, ex:
        logging.error("could not connect to MIS on %s: %s" % (MIS_HOST, ex))
        sys.exit(EXIT_ERROR)
        
    logging.info("Connected to MIS.")

    # Get (and print) information on the MIS server
    #mis.get_peer_info()
    peer_info_response = mis.get_peer_info()
    peer_info = {"response":peer_info_response[0], "node_id":peer_info_response[1], "sw_version":peer_info_response[2]}

    # assemble request
    request_payload = mis._new_request(0)
    #MAXMOD what do the following 2 lines mean to MIS?
    # ans: nothing! So I can use them for subband_selection
    #request_payload.append_bitset(512, range(512));
    #request_payload.append_string("START")
    #MAXMOD try to attach a subband (2bytes)
    request_payload.append_number(options.subband,2)
    
    # send request
    mis.send_packet(0x4f10, request_payload)
    
    # receive response (#1)
    (protocol, response_payload) = mis.recv_packet()
    
    # handle response (#1)
    
    assert protocol == 0x8f11
    
    response = {}
    response["seqnr"]        = response_payload.peel_number(8) # seqnr
    response["replynr"]      = response_payload.peel_number(8) # replynr
    response["peertime_ns"]  = response_payload.peel_timeval2ns()
    response["response"]     = response_payload.peel_string()
    
    logging.debug("MIS response (xcstats) {")
    logging.debug("    seqnr          " + str(response["seqnr"]))
    logging.debug("    replynr        " + str(response["replynr"]))
    logging.debug("    peertime_ns    " + str(ns2dt(response["peertime_ns"])))
    logging.debug("    response       " + str(response["response"]))
    
    if response["response"] != "ACK":
        logging.debug("} MIS response (XCStats)")
        logging.info("did not receive ACK from MIS.")
            
    else:
        response["payload_ns"]   = response_payload.peel_timeval2ns()
        response["invalid"]      = response_payload.peel_number_array(bytes_per_element = 4, as_string = True)
        response["rcu_settings"] = response_payload.peel_number_array(bytes_per_element = 4, as_string = True)
        response["subband"]      = response_payload.peel_number(2)
        response["geoloc"]       = response_payload.peel_number_array(bytes_per_element = 8, as_string = True)
        response["antcoord"]     = response_payload.peel_number_array(bytes_per_element = 8, as_string = True)
        response["acmdata"]      = response_payload.peel_number_array(bytes_per_element = 16, as_string = True)
        # acm vals are complex = 2 x 8 bytes. I'm stuck with 16bits at the moment because of the acmNOE that preceeds acm

        assert (len(response_payload)==0)

        response["station_id"]   = MIS_SI_ID
        response["time_string"]  = str(ns2dt(response["payload_ns"]))

        invalid_array            = numpy.fromstring(response["invalid"], dtype=numpy.int32)
        rcu_settings_array       = numpy.fromstring(response["rcu_settings"], dtype=numpy.int32)
        geoloc_array             = numpy.fromstring(response["geoloc"], dtype=numpy.float64)
        antcoord_array           = numpy.fromstring(response["antcoord"], dtype=numpy.float64)
        
        # real,imag,r,i...
        response["acm"]          = numpy.fromstring(response["acmdata"], dtype=numpy.float64, count=len(response["acmdata"])//8)
        response["rcu_settings_string"] = '{' + str.join(",", [("%d" % f) for f in rcu_settings_array]) + '}'
        #subband is scalar now
        response["subband_string"] = '{' + str(response["subband"]) + '}' 
        response["acm_string"] = '{' + str.join(",", [("%.6f" % f) for f in response["acm"]]) + '}'

        response["geoloc_string"]   = '{' + str.join(",", [("%.8f" % f) for f in geoloc_array]) + '}'
        response["antcoord_string"] = '{' + str.join(",", [("%.8f" % f) for f in antcoord_array]) + '}'

        logging.debug("    seqnr          " + str(response["seqnr"]))
        logging.debug("    replynr        " + str(response["replynr"]))
        logging.debug("    peertime_ns    " + str(ns2dt(response["peertime_ns"])))
        logging.debug("    payload_ns     " + str(ns2dt(response["payload_ns"])))
        logging.debug("    invalid        " + str([("x%08x"%(int(v)&0xFFFFFFFF)) for v in invalid_array]))
        logging.debug("    rcu_settings   " + str([("x%08x"%(int(v)&0xFFFFFFFF)) for v in rcu_settings_array]))
        logging.debug("    subband        " + str(response["subband"]))
        logging.debug("    geoloc         " + str([("%.8f"%v) for v in geoloc_array]))
        logging.debug("    antcoord       " + str([("%.8f"%v) for v in antcoord_array]))
        logging.debug("    acmdata length " + str(len(response["acmdata"])))
        logging.debug("} MIS response (XCStats)")        
        
        #MAXMOD debugging
        #fout = open('acmdata.out',"wb")
        #fout.write(response["acmdata"][16:])
        #fout.close()
       
        logging.info("received ACK.")

        if options.jobctrl:
            # ----- store information in the database -----
            # start a transaction
            db.begin_transaction()
                
        #classify
        N = int(numpy.sqrt(len(response["acm"])/2.)/2.)
        xst = xcstats_classifier_max_v2.unflatten(response["acm"],N)
        
        # serious debugging
        #f = open("_tmp_pickle.dat","wb")
        #pickle.dump(xst,f)
        #f.close()        
        
        #classification = xcstats_classifier_max.classify(xst,rcu_settings_masks.rcu_status(list(rcu_settings_array)))
        classification = xcstats_classifier_max_v2.classify(xst,list(rcu_settings_array))
        # print classification

        response["classification"] = '\'{' + str.join(",", classification) + '}\''
        logging.debug("    classification    " + response["classification"])

        query = "INSERT INTO lofar.antennacorrelationmatrices(time, si_id, rcu_settings, subband, geo_loc, ant_coord, acm_data, classification) VALUES (TIMESTAMP WITH TIME ZONE '%(time_string)s UTC', '%(station_id)s', '%(rcu_settings_string)s', '%(subband_string)s', '%(geoloc_string)s','%(antcoord_string)s', '%(acm_string)s', %(classification)s);" % response
        #print query[-200:]+'...'
        # print "len = ",len(antcoord_array),"\n",response["antcoord_string"]
        
        if options.commit:
            try:
                db.perform_query(query)
            except lofar.shm.DatabaseError, ex:
                db.rollback()
                logging.error("could not insert data: %s (%s)" % (query, ex))
                db.close()
                mis.disconnect()
                sys.exit(EXIT_ERROR)
           
        #put this back in before running for real
        if options.jobctrl:
            try:
                # if the ticket number is no longer valid, or a timeout has occurred, conditional_commit()
                # will RAISE an exception (which will cause immediate release of the lock it acquired).
                # otherwise a commit will be performed.
                
                jobctrl.conditional_commit(job)
            except lofar.shm.DatabaseError, ex:
                # reset connection state
                db.rollback()
                    
                # log error and exit
                logging.error("commit failed! (%s)" % (ex,))
                db.close()
           
                mis.disconnect()
                sys.exit(EXIT_ERROR)
           
#MAXMOD wht is this about resetting watchdog? not in fetch-sbstats
#                try:
#                    result = jobctrl.reset_watchdog(job)
#                except lofar.shm.DatabaseError, ex:
#                    result = False
#               
#                if not result:
#                    # log error and exit
#                   logging.error("could not reset watchdog (%s)" % (ex,))
#                   db.close()
#           
#                    mis.disconnect()
#                    sys.exit(EXIT_ERROR)
   
    # disconnect
    logging.info("Closing MIS connection...")
    mis.disconnect()
    
    db.close()
  
    print "Thank you for using the LOFAR-SHM Antenna Correlation Matrix Fetcher Daemon."


if __name__ == "__main__":
    main()
