#!/usr/bin/python2.4

import os
import sys
import time
import logging
import lofar.shm
import optparse
import misconnection
import numpy
#import sbstats_classifier_max_new
import sbstats_classifier_max_v2
from lofar.shm.nanostamp import ns2dt, now2ns
import socket
import re

EXIT_ERROR = 127

def main():
    parser = optparse.OptionParser(usage="%prog [options] host\n")
    parser.add_option("-j", "--job-control", action="store_true", dest="jobctrl", default=False, help="run this program under LOFAR job control")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="write debug information to syslog (default: false)")
    parser.add_option("-c", "--commit", action="store_true", dest="commit", default=False, help="commit SST data to SHM database (default: false)")
    parser.add_option("-p", "--port", dest="mis_port", type='int', help="port to query on MIS host (default: from SHM database)")
    (options, args) = parser.parse_args()

    print "This is the LOFAR-SHM Subband Statistics Fetcher Daemon, v1.1"

    if len(args) != 1:
        parser.print_help()
        sys.exit(EXIT_ERROR)

    # open connection to shm database
    db = lofar.shm.db.SysHealthDatabase()
    try:
        db.open()
    except lofar.shm.DatabaseError:
        sys.exit(EXIT_ERROR)

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

    logging.info("Connecting to MIS on %(MIS_HOST)s..." % vars())
    mis = misconnection.MISConnection(MIS_HOST, MIS_PORT)
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
    
    # send request
    mis.send_packet(0x4f0e, request_payload) # subband request
    
    # receive response
    (protocol, response_payload) = mis.recv_packet()
    
    # interpret response
    assert protocol == 0x8f0f
    
    response = {}
    response["seqnr"]        = response_payload.peel_number(8) # seqnr
    response["replynr"]      = response_payload.peel_number(8) # replynr
    response["peertime_ns"]  = response_payload.peel_timeval2ns()
    response["response"]     = response_payload.peel_string()

    logging.debug("MIS response (sbstats) {")
    logging.debug("    seqnr          " + str(response["seqnr"]))
    logging.debug("    replynr        " + str(response["replynr"]))
    logging.debug("    peertime_ns    " + str(ns2dt(response["peertime_ns"])))
    logging.debug("    response       " + response["response"])

    if response["response"] != "ACK":
        logging.debug("} MIS response (sbstats)")
        logging.info("did not receive ACK from MIS.")
            
    else:
        response["payload_ns"]   = response_payload.peel_timeval2ns()
        response["invalid"]      = response_payload.peel_number_array(bytes_per_element = 4, as_string = True)
        response["rcu_settings"] = response_payload.peel_number_array(bytes_per_element = 4, as_string = True)
        response["data"]         = response_payload.peel_number_array(bytes_per_element = 8, as_string = True)
    
        assert len(response_payload) == 0

        # handle response

        # response["station_id"] = 501
        # response["station_id"] = 1000 + int(re.search('\d+',peer_info["node_id"]).group(0))
        response["station_id"] = MIS_SI_ID
        response["time_string"] = str(ns2dt(response["payload_ns"]))
        invalid_array           = numpy.fromstring(response["invalid"], dtype=numpy.int32)
        rcu_settings_array      = numpy.fromstring(response["rcu_settings"], dtype=numpy.int32)
        #invalid_array           = numpy.asarray(response["invalid"], dtype=numpy.int32)
        #rcu_settings_array      = numpy.asarray(response["rcu_settings"], dtype=numpy.int32)
        num_rcu                 = len(rcu_settings_array)
        #spectrum                = numarray.fromstring(response["data"], numarray.Float64, [num_rcu,512])
        spectrum                = numpy.fromstring(response["data"],dtype=numpy.float64).reshape(num_rcu,512)
        assert len(response["data"])    == 8*512*num_rcu # sizeof(double) * 512 * #RCUs

        logging.info("received ACK.")

        logging.debug("    payload_ns     " + str(ns2dt(response["payload_ns"])))
        #logging.debug("    invalid        " + str([("%08x"%f) for f in response["invalid"]]))
        #logging.debug("    rcu_settings   " + str([("%08x"%f) for f in response["rcu_settings"]]))
        logging.debug("    invalid        " + str([("x%08x"%(int(v)&0xFFFFFFFF)) for v in invalid_array]))
        logging.debug("    rcu_settings   " + str([("x%08x"%(int(v)&0xFFFFFFFF)) for v in rcu_settings_array]))
        logging.debug("    data           " + str(len(response["data"])))
        logging.debug("} MIS response (sbstats)")
        
        if options.jobctrl:
            # ----- store information in the database -----
            # start a transaction
            db.begin_transaction()

        for rcu_id in range(num_rcu):
            # invalid_flags   = response_invalid[rcu_id]
            response["rcu_id"] = rcu_id
            # response["this_rcu_settings"] = response["rcu_settings"][rcu_id]
            response["this_rcu_settings"] = rcu_settings_array[rcu_id]
            response["spectrum_string"] = '{' + str.join(",", [("%.1f" % f) for f in spectrum[rcu_id,:]]) + '}'

            #(classification, response["median_power"], response["peak_power"], response["median_chan"]) = sbstats_classifier_max_new.classify(spectrum[rcu_id,:])
            # print "RCU #%02d  status=%x  len(spectrum)=%d"%(rcu_id,int(response["this_rcu_settings"])&0xFFFFFFFF,len(spectrum[rcu_id,:]))
            classification_result = sbstats_classifier_max_v2.classify(spectrum[rcu_id,:],response["this_rcu_settings"])
            logging.debug("    classification    " + str(classification_result[0])) 
            # print "classification_result:",classification_result
            if (len(classification_result) == 4):
                (classification, response["median_power"], response["peak_power"], response["median_chan"]) = classification_result
                response["classification"] = '\'{' + str.join(",", classification) + '}\''
                query = "INSERT INTO Lofar.SubbandStatistics(time, si_id, rcu_id, rcu_settings, spectrum, median_power, peak_power, median_chan, classification) VALUES (TIMESTAMP WITH TIME ZONE '%(time_string)s UTC', '%(station_id)s', %(rcu_id)i, %(this_rcu_settings)i, '%(spectrum_string)s', %(median_power)e, %(peak_power)e, %(median_chan)i, %(classification)s);" % response

            elif (len(classification_result) == 6):
                (classification, response["median_power"], response["peak_power"], response["median_chan"], fit_parameters, fitness) = classification_result
                response["classification"] = '\'{' + str.join(",", classification) + '}\''
                response["fit_parameters"] = '\'{' + str.join(",", [("%.6f" % f) for f in fit_parameters]) + ",%.8f"%fitness + '}\''
                query = "INSERT INTO Lofar.SubbandStatistics(time, si_id, rcu_id, rcu_settings, spectrum, median_power, peak_power, median_chan, fit_parameters, classification) VALUES (TIMESTAMP WITH TIME ZONE '%(time_string)s UTC', '%(station_id)s', %(rcu_id)i, %(this_rcu_settings)i, '%(spectrum_string)s', %(median_power)e, %(peak_power)e, %(median_chan)i, %(fit_parameters)s, %(classification)s);" % response

            if options.commit:
                try:
                    db.perform_query(query)
                except lofar.shm.DatabaseError, ex:
                    db.rollback()
                    logging.error("could not insert data: %s (%s)" % (query, ex))
                    db.close()
                    mis.disconnect()
                    sys.exit(EXIT_ERROR)

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

    # disconnect
    logging.info("Closing MIS connection...")
    db.close()

    mis.disconnect()
    print "Thank you for using the LOFAR-SHM Subband Statistics Fetcher Daemon."


# main program
if __name__ == "__main__":
    main()
