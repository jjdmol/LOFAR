#!/usr/bin/python

import os
import sys
import time
import datetime
import logging
import lofar.shm
import optparse
import misconnection
import socket
import numpy, string
import array
import re
import struct
import diagnosis_utils
from lofar.shm.nanostamp import ns2dt, now2ns

EXIT_ERROR = 127

STATUSLEN = 200

COMPONENT_STATE_OFF           = 0 
COMPONENT_STATE_OPERATIONAL   = 1
COMPONENT_STATE_MAINTENANCE   = 2
COMPONENT_STATE_TEST          = 3
COMPONENT_STATE_SUSPICIOUS    = 4 
COMPONENT_STATE_BROKEN        = 5

def main():

    parser = optparse.OptionParser(usage="%prog [options] si_id\n")
    parser.add_option("-j", "--job-control", action="store_true", dest="jobctrl", default=False, help="run this program under LOFAR job control")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="write debug information to syslog (default: false)")
    parser.add_option("-r", "--report", action="store_true", dest="report", default=False, help="report diagnoses to MIS hosts (default: false)")
    parser.add_option("-p", "--port", dest="mis_port", type='int', help="port to query on MIS host (default: from SHM database)")
    (options, args) = parser.parse_args()

    print "This is the LOFAR-SHM Diagnosis Calculator v2.0"
    
    if len(args) != 1:
        parser.print_help()
        sys.exit(EXIT_ERROR)

    if options.verbose:
        log_level = logging.DEBUG
    else:
        log_level = logging.INFO

    # open connection to shm database
    db = lofar.shm.db.SysHealthDatabase()
    try:
        db.open()
    except lofar.shm.DatabaseError:
        sys.exit(EXIT_ERROR)
       
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

    # get some diagnoses from the classifications
    # get sites from macinformationservers table 
    query = "SELECT * FROM lofar.macinformationservers WHERE (si_id = %d);" % MIS_SI_ID
    logging.debug("Querying SHM database for MIS hosts...")
    try:
        dbres = db.perform_query(query)
    except lofar.shm.DatabaseError, ex:
        logging.error("could not query data: %s (%s)" % (query, ex))
        db.close()
        sys.exit(EXIT_ERROR)

    # initially intended to diagnose multiple sites; now expect single site give as arg[0].
    # might as well keep multi-site loop.

    sites = []
    for i in range(len(dbres)):
        sites.append({'id' : dbres[i].si_id, 'name' : dbres[i].si_name, 'inet' : dbres[i].mis_address, 'port':dbres[i].mis_port})

    for site_dict in sites:
        # come up with a consistent, simple, diagnosis based on most recent data,
        # then insert it into diagnosis table
        diagnosis = {'time':datetime.datetime.utcnow(),'num_faults':0,'datum_epoch':[],'component':[], 'state':[], 'confidence':[], 'url':[], 'reported':'false'}

        si_id = site_dict['id']
        si_name = site_dict['name']
        diagnosis['si_id'] = si_id

        # get most recent full-site SST dataset
        query = "SELECT time FROM lofar.subbandstatistics WHERE (si_id = %(si_id)d) AND (rcu_id = 0) ORDER BY TIME DESC LIMIT 2;" % vars()
        try:
            dbres = db.perform_query(query)
        except lofar.shm.DatabaseError, ex:
            logging.error("could not query data: %s (%s)" % (query, ex))
            db.close()
            sys.exit(EXIT_ERROR)

        #in case of nearly empty database
        if (len(dbres) > 0):
            tb = dbres[0].time
            if (len(dbres) == 1):
                ta = tb - datetime.timedelta(1) 
            else:                
                ta = dbres[1].time

            logging.debug("Querying SHM database for latest SST entries...")
            logging.debug("SST: %(si_id)d : ta=%(ta)s tb=%(tb)s"%vars())
            # query = "SELECT * from lofar.subbandstatistics WHERE (si_id = %(si_id)d) and (time > '%(ta)s') and (time <= '%(tb)s') order by rcu_id;" % vars()
            query = "SELECT * from lofar.subbandstatistics WHERE (si_id = %(si_id)d) and (time > '%(ta)s') and (time <= '%(tb)s') and "\
                    "(classification[1] <> 'NOMINAL')  order by rcu_id;" % vars()
            try:
                sstdbres = db.perform_query(query)
            except lofar.shm.DatabaseError, ex:
                logging.error("could not query data: %s (%s)" % (query, ex))
                db.close()
                sys.exit(EXIT_ERROR)

            Nsst = len(sstdbres)
        else:
            Nsst = 0
        
        # get XSTs
        query = "SELECT time FROM lofar.antennacorrelationmatrices WHERE (si_id = %(si_id)d) ORDER BY TIME DESC LIMIT 2;" % vars()
        try:
            dbres = db.perform_query(query)
        except lofar.shm.DatabaseError, ex:
            logging.error("could not query data: %s (%s)" % (query, ex))
            db.close()
            sys.exit(EXIT_ERROR)

        #in case of nearly empty database
        if (len(dbres) > 0):
            tb = dbres[0].time
            if (len(dbres) == 1):
                ta = tb - datetime.timedelta(1) 
            else:                
                ta = dbres[1].time

            logging.debug("Querying SHM database for latest XST entries...")
            logging.debug("XST: %(si_id)d : ta=%(ta)s tb=%(tb)s"%vars())
            # query = "SELECT * from lofar.antennacorrelationmatrices WHERE (si_id = %(si_id)d) AND (time > '%(ta)'s) AND (time <= '%(tb)s');" % vars()
            query = "SELECT * from lofar.antennacorrelationmatrices WHERE (si_id = %(si_id)d) AND (time > '%(ta)s') AND (time <= '%(tb)s') and 'OFF_NOMINAL' = any (classification);" % vars()
            try:
                xstdbres = db.perform_query(query)
            except lofar.shm.DatabaseError, ex:
                logging.error("could not query data: %s (%s)" % (query, ex))
                db.close()
                sys.exit(EXIT_ERROR)
            Nxst = len(xstdbres)
        else:
            Nxst = 0

        # get RSPs
        query = "SELECT time FROM lofar.rspstatus WHERE (si_id = %(si_id)d) and (rsp_id = 0) ORDER BY TIME DESC LIMIT 2;" % vars()
        try:
            dbres = db.perform_query(query)
        except lofar.shm.DatabaseError, ex:
            logging.error("could not query data: %s (%s)" % (query, ex))
            db.close()
            sys.exit(EXIT_ERROR)

        #in case of nearly empty database
        if (len(dbres) > 0):
            tb = dbres[0].time
            if (len(dbres) == 1):
                ta = tb - datetime.timedelta(1) 
            else:                
                ta = dbres[1].time

            logging.debug("Querying SHM database for latest RSP entries...")
            logging.debug("RSP: %(si_id)d : ta=%(ta)s tb=%(tb)s"%vars())
            # query = "SELECT * from lofar.antennacorrelationmatrices WHERE (si_id = %(si_id)d) AND (time > '%(ta)'s) AND (time <= '%(tb)s');" % vars()
            query = "SELECT * from lofar.rspstatus WHERE (si_id = %(si_id)d) AND (time > '%(ta)s') AND (time <= '%(tb)s');" % vars()
            try:
                rspdbres = db.perform_query(query)
            except lofar.shm.DatabaseError, ex:
                logging.error("could not query data: %s (%s)" % (query, ex))
                db.close()
                sys.exit(EXIT_ERROR)
            Nrsp = len(rspdbres)
        else:
            Nrsp = 0
        
        # now create diagnosis based on data classifications
        disregard_rcus = numpy.ones(Nsst)
        for i in range(Nsst):
            # create a fault report
            if ( (sstdbres[i].classification[0] == 'ZEROES') or
                 (sstdbres[i].classification[0] == 'FLAT') or
                 (sstdbres[i].classification[0] == 'SHIFT_LEFT') or
                 (sstdbres[i].classification[0] == 'SHIFT_RIGHT') or
                 (sstdbres[i].classification[0] == 'INDETERMINATE_RCU_STATUS') or
                 (sstdbres[i].classification[0] == 'HBA_FILTERS_OFF') or
                 (sstdbres[i].classification[0] == 'INVERTED') or
                 (sstdbres[i].classification[0] == 'RCUSTATUS_WRONG_BAND') or
                 (sstdbres[i].classification[0] == 'LOW_AMP') or
                 (sstdbres[i].classification[0] == 'OFF_NOMINAL') ):

                # for later use
                disregard_rcus[i] = 0.0;

                rcu_id = sstdbres[i].rcu_id
                rsp_id = rcu_id / 8
                subrack_id = rsp_id / 4
                cabinet_id = subrack_id / 2
                component = '%(si_name)s:LOFAR_PIC_Cabinet%(cabinet_id)d_Subrack%(subrack_id)d_RSPBoard%(rsp_id)d_RCU%(rcu_id)d' % vars()

                # improve this!
                state = COMPONENT_STATE_SUSPICIOUS  
                
                # improve this!
                certainty = 7501 # see MIS_Protocol.prot

                # make url -- encode data time
                url_string = 'data/sst/%s/image/%d/?epoch=%s.html'%(si_name.lower(),rcu_id,sstdbres[i].time.strftime("%Y-%m-%d+%H%%3A%M%%3A%S"))
                # print url_string
                # diagnosis.add((component,state,certainty,url_string))
                diagnosis['component'].append(component)
                diagnosis['state'].append(state)
                diagnosis['confidence'].append(certainty)
                diagnosis['datum_epoch'].append(sstdbres[i].time)
                diagnosis['url'].append(url_string)
                logging.debug("SST fault detected:  rcu=%02d, classification=%s"%(sstdbres[i].rcu_id,sstdbres[i].classification))
                                
        for i in range(Nxst):
            for j in range(len(xstdbres[i].classification)):
                if ( ((xstdbres[i].classification[j] == 'INVALID') or
                      (xstdbres[i].classification[j] == 'OFF_NOMINAL')) and
                     (disregard_rcus[j]             ==  0.0) ):
                    rcu_id = j
                    rsp_id = rcu_id / 8
                    subrack_id = rsp_id / 4
                    cabinet_id = subrack_id / 2
                    component = '%(si_name)s:LOFAR_PIC_Cabinet%(cabinet_id)d_Subrack%(subrack_id)d_RSPBoard%(rsp_id)d_RCU%(rcu_id)d' % vars()

                    # improve this!
                    state = COMPONENT_STATE_SUSPICIOUS  
                    
                    # improve this!
                    certainty = 7501 # see MIS_Protocol.prot
                    
                    # make url
                    url_string = 'data/xst/%s/image/0/?epoch=%s.html'%(si_name.lower(),xstdbres[i].time.strftime("%Y-%m-%d+%H%%3A%M%%3A%S"))

                    # diagnosis.add((component,state,certainty,url_string))
                    diagnosis['component'].append(component)
                    diagnosis['state'].append(state)
                    diagnosis['confidence'].append(certainty)
                    diagnosis['datum_epoch'].append(xstdbres[i].time)
                    diagnosis['url'].append(url_string)
                    #diagnosis['datum_epoch'].append(xstdbres[i].time.strftime("%Y-%m-%d %H:%M:%S"))

                    logging.debug("XST fault detected:  rcu=%02d, classification=%s"%(j,xstdbres[i].classification[j]))
                    
        for i in range(Nrsp):
            if (len(rspdbres[i].classification)>0):
                # if (re.match(rspdbres.classification[0],"OFF_NOM*")):
                if (rspdbres[i].classification[0] != 'NOMINAL'):
                    rsp_id = i
                    subrack_id = rsp_id / 4
                    cabinet_id = subrack_id / 2
                    component = '%(si_name)s:LOFAR_PIC_Cabinet%(cabinet_id)d_Subrack%(subrack_id)d_RSPBoard%(rsp_id)d' % vars()
                    # .AP2 for eacample
                    # improve this!
                    state = COMPONENT_STATE_SUSPICIOUS  
            
                    # improve this!
                    certainty = 7501 # see MIS_Protocol.prot
            
                    # make url
                    url_string = 'data/rsp/%s/image/%d/?epoch=%s.html'%(si_name.lower(),rsp_id,rspdbres[i].time.strftime("%Y-%m-%d+%H%%3A%M%%3A%S"))

                    # diagnosis.add((component,state,certainty,url_string))
                    diagnosis['component'].append(component)
                    diagnosis['state'].append(state)
                    diagnosis['confidence'].append(certainty)
                    diagnosis['datum_epoch'].append(rspdbres[i].time)
                    diagnosis['url'].append(url_string)
                    #diagnosis['datum_epoch'].append(rspdbres[i].time.strftime("%Y-%m-%d %H:%M:%S"))
                    
        # compare current diagnosis to most recent db entry, if different commit this one.
        # flattten to a set for easy comparison
        current_fault_set = diagnosis_utils.diagnosis_to_set(diagnosis)

        query = "SELECT * from lofar.diagnoses WHERE (si_id = %(si_id)d) "\
                "ORDER by time desc limit 1;" % (vars())

        try:
            diagdbres = db.perform_query(query)
        except lofar.shm.DatabaseError, ex:
            logging.error("could not query data: %s (%s)" % (query, ex))
            db.close()
            sys.exit(EXIT_ERROR)

        if (len(diagdbres) == 0):
                last_fault_set = set()
        else:
            db_diagnosis = {'time'        : diagdbres[0].time,
                            'si_id'       : diagdbres[0].si_id,
                            'num_faults'  : diagdbres[0].num_faults,
                            'datum_epoch' : diagdbres[0].datum_epoch,
                            'component'   : diagdbres[0].component,
                            'state'       : diagdbres[0].state,
                            'confidence'  : diagdbres[0].confidence,
                            'reported'    : diagdbres[0].reported_to_mis}
            last_fault_set = diagnosis_utils.diagnosis_to_set(db_diagnosis)
            
        # clunky here. Can this logic be streamlined?
        new_state = False # new diagnoses should be committed to database
        if (len(current_fault_set) != len(last_fault_set)):
            new_state = True
        else:
            if len((current_fault_set | last_fault_set) - current_fault_set - last_fault_set) > 0:
                # diagnoses are different
                new_state = True

        if (new_state):
            logging.info("New fault state detected on %s at %s"%(si_name,diagnosis['time']))
        else:
            logging.info("No change in fault state detected on %s at %s"%(si_name,diagnosis["time"]))
            
        # commit to diagnosis table
        if (new_state):
            commits = {}
            commits["time"] = "%s" % diagnosis["time"]
            commits["si_id"] = "%d" % si_id
            commits["num_faults"] = "%d" % len(current_fault_set)
            commits["component"] = '{' + str.join(",", [("%s" % s) for s in diagnosis["component"]]) + '}'
            commits["url"] = '{' + str.join(",", [("%s" % s) for s in diagnosis["url"]]) + '}'
            commits["state"] = '{' + str.join(",", [("%d" % s) for s in diagnosis["state"]]) + '}'
            commits["datum_epoch"] = '{' + str.join(",", [("%s" % s) for s in diagnosis["datum_epoch"]]) + '}'
            commits["confidence"] = '{' + str.join(",", [("%d" % s) for s in diagnosis["confidence"]]) + '}'
            commits["reported_to_mis"] = "false"
            
            query = "INSERT INTO lofar.diagnoses (time, si_id, num_faults, component, url, state, confidence, datum_epoch, reported_to_mis) VALUES "\
                    "(TIMESTAMP WITH TIME ZONE '%(time)s UTC', %(si_id)s, %(num_faults)s, '%(component)s', '%(url)s', "\
                    "'%(state)s', '%(confidence)s', '%(datum_epoch)s', %(reported_to_mis)s);" % commits
            #print query
            
            try:
                db.perform_query(query)
            except lofar.shm.DatabaseError, ex:
                db.rollback()
                #logging.error("could not insert data: %s (%s)" % (query, ex))
                logging.error("could not insert data: %s" % (ex))
                db.close()
                sys.exit(EXIT_ERROR)

        # if report option on, report diagnosis to MIS host
        # special case: if diagnoses are same but last diagnosis was not reported, then report now
        do_report = False
        if options.report:
            if new_state:
                do_report = True
            else:
                if (len(last_fault_set) > 0):
                    if not db_diagnosis["reported"]:
                        do_report = True

        if do_report:
            logging.info("Will report diagnosis of station %(name)s" % site_dict)
            logging.info("Connecting to MIS on %(name)s port %(port)d..." % site_dict)
            mis = misconnection.MISConnection(site_dict["inet"], site_dict["port"])
            # has initialized 
            try:
                mis.connect()
            except socket.error, ex:
                logging.error("could not connect to MIS on %s: %s" % (site_dict["name"], ex))
                sys.exit(EXIT_ERROR)
        
                logging.info("Connected to MIS.")

            # Get (and print) information on the MIS server
            peer_info_response = mis.get_peer_info()
            peer_info = {"response":peer_info_response[0], "node_id":peer_info_response[1], "sw_version":peer_info_response[2]}
            if peer_info["response"] != 'ACK' :
                logging.error("Received %s from MIS on %s; expected ACK" % (peer_info["response"],site_dict["name"]))
                mis.disconnect()
                sys.exit(EXIT_ERROR)
            
            # defined in misconnection
            # do_diagnosis_response = mis.do_diagnosis("LOFAR_PIC_Cabinet0_Subrack0_RSPBoard1_RCU8",5,2,'http://10.230.30.1/shm/data')
            
            for i in range(len(diagnosis["component"])):
                do_diagnosis_response = mis.do_diagnosis(diagnosis["component"][i],
                                                         diagnosis["state"][i],
                                                         diagnosis["confidence"][i],
                                                         diagnosis_utils.SHM_WEB_PREF + diagnosis["url"][i])
                if do_diagnosis_response != 'ACK' :
                    logging.error("Received %s from MIS on %s upon DiagnosisNotification; expected ACK" % (do_diagnosis_response,site_dict["name"]))
                    mis.disconnect()
                    sys.exit(EXIT_ERROR)

                # slow things down for MIS server -- seems to need it.
                time.sleep(0.25)

            # update diagnosis table 'reported' flag
            # print "diagnosis : \n %s"%diagnosis
            # print "\n db_diagnosis : \n %s"%db_diagnosis
            if (new_state):
                query = "UPDATE lofar.diagnoses SET reported_to_mis = true WHERE (si_id = %(si_id)s) AND (time = '%(time)s+00');" % diagnosis
            else:
                query = "UPDATE lofar.diagnoses SET reported_to_mis = true WHERE (si_id = %(si_id)s) AND (time = '%(time)s');" % db_diagnosis
            try:
                db.perform_query(query)
            except lofar.shm.DatabaseError, ex:
                db.rollback()
                logging.error("could not update data: %s (%s)" % (query, ex))
                db.close()
                sys.exit(EXIT_ERROR)
            

            # disconnect
            logging.info("Closing MIS connection...")
            mis.disconnect()
    
    print "Thank you for using the LOFAR-SHM Diagnosis Report Script."



if __name__ == "__main__":
    main()
