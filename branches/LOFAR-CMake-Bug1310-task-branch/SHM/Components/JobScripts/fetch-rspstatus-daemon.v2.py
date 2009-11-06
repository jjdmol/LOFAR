#!/usr/bin/python

import os
import sys
import time
import logging
import lofar.shm
import optparse
import misconnection
import socket
import rspstatus_classifier
import numpy, string
import array
import re
import struct

from lofar.shm.nanostamp import ns2dt, now2ns

EXIT_ERROR = 127

STATUSLEN = 200

def main():

    parser = optparse.OptionParser(usage="%prog [options] host\n")
    parser.add_option("-j", "--job-control", action="store_true", dest="jobctrl", default=False, help="run this program under LOFAR job control")
    parser.add_option("-c", "--commit", action="store_true", dest="commit", default=False, help="commit RSP data to SHM database (default: false)")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="write debug information to syslog (default: false)")
    parser.add_option("-p", "--port", dest="mis_port", type='int', help="port to query on MIS host (default: from SHM database)")
    (options, args) = parser.parse_args()

    print "This is the LOFAR-SHM RSP Board Status Fetcher v1.1"
    
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

    logging.info("Connecting to MIS on %(MIS_HOST)s port %(MIS_PORT)s..." % vars())
    mis = misconnection.MISConnection(MIS_HOST, MIS_PORT)
    #has initialized 
    try:
        mis.connect()
    except socket.error, ex:
        logging.error("could not connect to MIS on %s: %s" % (MIS_HOST, ex))
        sys.exit(EXIT_ERROR)
        
    logging.info("Connected to MIS.")

    # Get (and print) information on the MIS server
    peer_info_response = mis.get_peer_info()
    peer_info = {"response":peer_info_response[0], "node_id":peer_info_response[1], "sw_version":peer_info_response[2]}
    
    # assemble request
    request_payload = mis._new_request(0)

    # send request
    mis.send_packet(0x4f12, request_payload)
    
    # receive response (#1)
    (protocol, response_payload) = mis.recv_packet()
    
    # interpet response (#1)
    assert protocol == 0x8f13
    
    response = {}
    response["seqnr"]        = response_payload.peel_number(8) # seqnr
    response["replynr"]      = response_payload.peel_number(8) # replynr
    response["peertime_ns"]  = response_payload.peel_timeval2ns()
    response["response"]     = response_payload.peel_string()
    if response["response"] != "ACK":
        logging.debug("} MIS response (rspstatus)")
        logging.info("did not receive ACK from MIS.")
    else:
        response["payload_ns"]   = response_payload.peel_timeval2ns()
        response["statusdata"]   = response_payload.peel_number_array(bytes_per_element = STATUSLEN, as_string = True)

        assert len(response_payload)==0
        
        logging.debug("MIS response (rspstatus){")
        logging.debug("-- seqnr          " + str(response["seqnr"]))
        logging.debug("-- replynr        " + str(response["replynr"]))
        logging.debug("-- peertime_ns    " + str(ns2dt(response["peertime_ns"])))
        logging.debug("-- response       " + str(response["response"]))
        logging.debug("-- payload_ns     " + str(ns2dt(response["payload_ns"])))
        logging.debug("-- statusdata_len " + str(len(response["statusdata"])))
        
        # take station_id from peer-info, but presently wants to be an integer. kludge an integer for now
        #response["station_id"] = int(re.search('\d+',peer_info["node_id"]).group(0)) + 1000
        response["station_id"] = MIS_SI_ID
        # print "MAXMOD station_is ", response["station_id"]
    
        logging.info("received ACK.")
        
        if options.jobctrl:
            # ----- store information in the database -----
            # start a transaction
            db.begin_transaction()

        #assemble database insert query
        num_rsp = len(response["statusdata"])//STATUSLEN        

        response["time_string"] = str(ns2dt(response["payload_ns"]))

        #pluck out the components
        #See EPA_Protocol.ph for structure defns
        RSPStatus  = []
        ETHStatus  = []
        MEPStatus  = []
        DIAGStatus = []
        BSStatus   = []
        RCUStatus  = []
        RSUStatus  = []
        ADOStatus  = []
        for rsp_id in range(num_rsp):
            offset = rsp_id*STATUSLEN
            RSPStatus.append(struct.unpack('<10BH',response['statusdata'][0+offset:offset+12]))
            ETHStatus.append(struct.unpack('<2I4B',response['statusdata'][12+offset:offset+24]))
            MEPStatus.append(struct.unpack('<H2B',response['statusdata'][24+offset:offset+28]))
            DIAGStatus.append(struct.unpack('<2B11H',response['statusdata'][28+offset:offset+52]))
            BSStatus.append(struct.unpack('<16I',response['statusdata'][52+offset:offset+116]))
            RCUStatus.append(struct.unpack('<4B2I4B2I4B2I4B2I',response['statusdata'][116+offset:offset+164]))
            RSUStatus.append(struct.unpack('<4B',response['statusdata'][164+offset:offset+168]))
            ADOStatus.append(struct.unpack('<8i',response['statusdata'][168+offset:offset+200]))

            response["rsp_id"] = rsp_id
            v = RSPStatus[rsp_id]
            #cache for classification
            board_volts = [v[0] * 2.5/192., v[1] * 3.3/192., v[2] * 5.0/192.]
            response["RSP_board_volts_string"] = '{' + \
                                                 str.join(",",[("%5.2f"%f) for f in board_volts]) \
                                                 + '}'
            #cache
            board_temps = v[3:9]
            response["RSP_board_temps_string"] = '{' + str.join(",",[("%3d"%f) for f in board_temps]) + '}'
            response["RSP_bp_clock"]           = v[9]

            v = ETHStatus[rsp_id]
            response["ETH_num_frames"] = v[0]
            response["ETH_num_errors"] = v[1]
            response["ETH_last_error"] = v[2]

            v = MEPStatus[rsp_id]
            response["MEP_seqnr"] = v[0]
            response["MEP_error"] = v[1]
            
            v = DIAGStatus[rsp_id]
            response["DIAG_interface"]     = v[0]
            response["DIAG_mode"]           = v[1]
            response["DIAG_ri_errors"]     = v[2]
            response["DIAG_rcux_errors"]   = v[3]
            response["DIAG_rcuy_errors"]   = v[4]
            response["DIAG_lcu_errors"]    = v[5]
            response["DIAG_cep_errors"]    = v[6]
            response["DIAG_serdes_errors"] = v[7]
            response["DIAG_ap_ri_errors"]  = '{' + str.join(",",[("%u"%f) for f in v[8:12]]) + '}'

            v = BSStatus[rsp_id]
            response["BS_ext_count_string"]     = '{' + str.join(",", [("%u"%f) for f in [v[0],v[4],v[8],v[12]]]) + '}'
            response["BS_sync_count_string"]    = '{' + str.join(",", [("%u"%f) for f in [v[1],v[5],v[9],v[13]]]) + '}'
            response["BS_sample_offset_string"] = '{' + str.join(",", [("%u"%f) for f in [v[2],v[6],v[10],v[14]]]) + '}'
            response["BS_slice_count_string"]   = '{' + str.join(",", [("%u"%f) for f in [v[3],v[7],v[11],v[15]]]) + '}'
            
            v = RCUStatus[rsp_id]
            response["RCU_pllx_string"] = '{' + str.join(",", [("%u"%f) for f in [v[0] & 0x01, v[6] & 0x01, v[12] & 0x01, v[18] & 0x01]]) + '}'
            response["RCU_plly_string"] = '{' + str.join(",", [("%u"%f) for f in \
                                                               [(v[0]>>1) & 0x01, (v[6]>>1) & 0x01, (v[12]>>1) & 0x01, (v[18]>>1) & 0x01]]) + '}'
            #cache
            rcu_num_overflow_x = [v[4],v[10],v[16],v[22]]
            rcu_num_overflow_y = [v[5],v[11],v[17],v[23]]
            response["RCU_num_overflow_x"] = '{' + str.join(",", [("%u"%f) for f in rcu_num_overflow_x]) + '}'
            response["RCU_num_overflow_y"] = '{' + str.join(",", [("%u"%f) for f in rcu_num_overflow_y]) + '}' 

            v = RSUStatus[rsp_id]
            response["RSU_ready_string"]      =  '{' + str.join(",", [("%u"%f) for f in \
                                                                      [v[0] & 0x01, v[1] & 0x01, v[2] & 0x01, v[3] & 0x01]]) + '}'
            response["RSU_error_string"]      =  '{' + str.join(",", [("%u"%f) for f in \
                                                                      [(v[0] >> 1) & 0x01, (v[1] >> 1) & 0x01, \
                                                                       (v[2] >> 1) & 0x01, (v[3] >> 1) & 0x01]]) + '}'
            response["RSU_apbp_string"]       =  '{' + str.join(",", [("%u"%f) for f in \
                                                                      [(v[0] >> 2) & 0x01, (v[1] >> 2) & 0x01, \
                                                                       (v[2] >> 2) & 0x01, (v[3] >> 2) & 0x01]]) + '}'
            response["RSU_image_type_string"] =  '{' + str.join(",", [("%u"%f) for f in \
                                                                      [(v[0] >> 3) & 0x01, (v[1] >> 3) & 0x01, \
                                                                       (v[2] >> 3) & 0x01, (v[3] >> 3) & 0x01]]) + '}'
            response["RSU_trig_string"]       =  '{' + str.join(",", [("%u"%f) for f in \
                                                                      [(v[0] >> 4) & 0x03, (v[1] >> 4) & 0x03, \
                                                                       (v[2] >> 4) & 0x03, (v[3] >> 4) & 0x03]]) + '}'
            v = ADOStatus[rsp_id]
            #cache
            ado_adc_offset_x = [v[0]/BSStatus[rsp_id][3]/4,  v[2]/BSStatus[rsp_id][7]/4,\
                                v[4]/BSStatus[rsp_id][11]/4, v[6]/BSStatus[rsp_id][15]/4]
            ado_adc_offset_y = [v[1]/BSStatus[rsp_id][3]/4,  v[3]/BSStatus[rsp_id][7]/4,\
                                v[5]/BSStatus[rsp_id][11]/4, v[7]/BSStatus[rsp_id][15]/4]

            response["ADO_adc_offset_x_string"] = '{' + str.join(",", [("%d"%f) for f in ado_adc_offset_x]) + '}'
            response["ADO_adc_offset_y_string"] = '{' + str.join(",", [("%d"%f) for f in ado_adc_offset_y]) + '}'

            #debugging output
            outkeys = [(re.search('RSP.+|ETH.+|MEP.+|DIAG.+|BS.+|RCU.+|RSU.+|ADO.+',k)) for k in response.keys()]
            for i in range(outkeys.count(None)):
                outkeys.remove(None)
            outkeys = [(k.group(0)) for k in outkeys]
            outkeys.sort()
            logging.debug("%-25s   %-20s"% ('rsp_id',response['rsp_id']))
            for k in outkeys:
                logging.debug("%-25s   %-20s"% (k,response[k]))

            #classify rspstatus params before they go into DB
            classification = []
            classification.append(rspstatus_classifier.classify_adcs(ado_adc_offset_x, ado_adc_offset_y))
            classification.append(rspstatus_classifier.classify_overflows(rcu_num_overflow_x, rcu_num_overflow_y))
            classification.append(rspstatus_classifier.classify_temps(board_temps))
            classification.append(rspstatus_classifier.classify_volts(board_volts))
            #clear out null strings
            for i in range(0, classification.count('')):
                classification.remove('')

            if (len(classification) == 0):
                classification.append('NOMINAL')

            classification_string = '\'{' + str.join(",", classification) + '}\''
            logging.debug("classification    " + ' '*10 + classification_string)

            #database insert query
            #this is dumb but I want to remove all the "_string"s from the keys to use as db row names.
            rows = [(re.sub('_string','',k)) for k in outkeys]
            query = "INSERT INTO Lofar.RSPStatus(time, si_id, rsp_id, "
            query = query + str.join(", ",rows) + ", classification) VALUES (TIMESTAMP WITH TIME ZONE '%(time_string)s UTC', '%(station_id)s', %(rsp_id)i, " % \
                    response
            query = query + str.join(", ",[("'%s'"%response[k]) for k in outkeys]) + ", " + classification_string + ");"

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
    mis.disconnect()
    
    print "Thank you for using the LOFAR-SHM RSP Status Fetch and Dump."


if __name__ == "__main__":
    main()
