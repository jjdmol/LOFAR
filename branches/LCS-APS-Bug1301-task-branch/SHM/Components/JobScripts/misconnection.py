#! /usr/bin/env python

import logging, sys

from lofar.shm.gcfconnection import GCFConnection, OutgoingByteList
from lofar.shm.nanostamp     import ns2dt, now2ns;

logging.basicConfig(level=logging.DEBUG, datefmt='%Y-%m-%d %H:%M:%S', format='%(asctime)s.%(msecs)03d %(levelname)-10s | %(message).70s')

class MISConnection(GCFConnection):

    #These defns are not used presently (protocol numbers explicitly set in call to mis.send_packet)
    MIS_PROTOCOL_ID = 15

    MIS_GENERIC_PINGPONG_ID                          =   1
    MIS_GENERIC_IDENTIFY_REQUEST_ID                  =   2
    MIS_GENERIC_IDENTIFY_RESPONSE_ID                 =   3
    MIS_DIAGNOSIS_NOTIFICATION_ID                    =   4
    MIS_DIAGNOSIS_RESPONSE_ID                        =   5
    MIS_RECONFIGURATION_REQUEST_ID                   =   6
    MIS_RECONFIGURATION_RESPONSE_ID                  =   7
    MIS_LOFAR_STRUCTURE_REQUEST_ID                   =   8
    MIS_LOFAR_STRUCTURE_RESPONSE_ID                  =   9
    MIS_LOFAR_STRUCTURE_CHANGED_ASYNC_ID             =  10
    MIS_PVSS_DP_SUBSCRIPTION_REQUEST_ID              =  11
    MIS_PVSS_DP_SUBSCRIPTION_RESPONSE_ID             =  12
    MIS_PVSS_DP_SUBSCRIPTION_VALUE_CHANGED_ASYNC_ID  =  13
    MIS_SUBBAND_STATISTICS_REQUEST_ID                =  14
    MIS_SUBBAND_STATISTICS_RESPONSE_ID               =  15
    MIS_ACM_SUBSCRIPTION_REQUEST_ID                  =  16
    MIS_ACM_SUBSCRIPTION_RESPONSE_ID                 =  17
    MIS_ACM_NOTIFICATION_ASYNC_ID                    =  18

    def __init__(self, host, port):
        GCFConnection.__init__(self, host, port)        
        self._request_seqnr = 0;

    def _new_request(self, replynr):
        # note that GCF preceded this packet with a 2-byte 'protocol' identifier
        # followed by a 4-byte 'payload length' in bytes.
        self._request_seqnr = self._request_seqnr + 1
        request_payload = OutgoingByteList()
        request_payload.request_seqnr = self._request_seqnr
        request_payload.ns = now2ns()
        # 8 byte request number
        request_payload.append_number(self._request_seqnr, 8)
        # 8 byte reply number
        request_payload.append_number(replynr, 8)
        # 12 byte timestamp
        request_payload.append_time_ns(request_payload.ns)
        return request_payload

    def do_ping(self):

        # assemble request
        request_payload = self._new_request(0);
        request_payload.append_number(1, 4)                 # TTL=1

        # send request
        self.send_packet(0xcf01, request_payload)           # send packet type 0xcf01

        # receive response
        (protocol, response_payload) = self.recv_packet()
        received_ns = now2ns()

        # interpret response

        assert protocol == 0xcf01

        response_seqnr       = response_payload.peel_number(8) # seqnr
        response_replynr     = response_payload.peel_number(8) # replynr
        response_echotime_ns = response_payload.peel_timeval2ns();
        response_ttl         = response_payload.peel_number(4)

        assert len(response_payload)==0

        # handle response
        return (request_payload.ns, response_echotime_ns, received_ns)

    def get_peer_info(self):

        # assemble request
        request_payload = self._new_request(0)

        # send request
        self.send_packet(0x4f02, request_payload) # id request

        # receive response
        # print "receiving response ..."
        (protocol, response_payload) = self.recv_packet()
        # print "received response ..."
        # interpret response

        assert protocol == 0x8f03

        response_seqnr       = response_payload.peel_number(8) # seqnr
        response_replynr     = response_payload.peel_number(8) # replynr
        response_echotime_ns = response_payload.peel_timeval2ns()
        response_response    = response_payload.peel_string()
        response_node_id     = response_payload.peel_string()
        response_sw_version  = response_payload.peel_string()

        assert len(response_payload)==0

        # handle response
        logging.info("MIS response (identify) {")
        logging.info("    seqnr          " + str(response_seqnr))
        logging.info("    replynr        " + str(response_replynr))
        logging.info("    echotime_ns    " + str(ns2dt(response_echotime_ns)))
        logging.info("    response       " + response_response)
        logging.info("    node_id        " + response_node_id)
        logging.info("    sw_version     " + response_sw_version)
        logging.info("} response (identify)")

        #MAXMOD
        return (response_response, response_node_id, response_sw_version)
    
    def do_diagnosis(self, component, diagnosis, confidence, diagnosis_id):

        # assemble request
        request_payload = self._new_request(0)
        request_payload.append_time_ns(now2ns()) # diagnosis time
        request_payload.append_number(diagnosis,8) # diagnosis
        request_payload.append_number(confidence, 2) # confidence
        request_payload.append_string(component) # component
        request_payload.append_string(diagnosis_id) # diagnosis id

        # send request
        self.send_packet(0x4f04, request_payload)
        
        # receive response
        (protocol, response_payload) = self.recv_packet()

        # assert protocol == 0x8f05
        assert protocol == 0x4f04
        # return response_payload
    
        response_seqnr        = response_payload.peel_number(8) # seqnr
        response_replynr      = response_payload.peel_number(8) # replynr
        response_peertime_ns  = response_payload.peel_timeval2ns()
        response_response     = response_payload.peel_string()

        assert len(response_payload)==0

        # handle response
        logging.info("MIS response (DiagnosisNotification) {")
        logging.info("   seqnr          " + str(response_seqnr))
        logging.info("   replynr        " + str(response_replynr))
        logging.info("   peertime_ns    " + str(ns2dt(response_peertime_ns)))
        logging.info("   response       " + str(response_response))

        return (response_response)

    def do_dp(self, dpname, mode = None):

        if mode is None:
            mode = "SINGLE-SHOT"

        assert mode in ["SINGLE-SHOT", "SUBSCRIBE", "UNSUBSCRIBE"]
        
        # assemble request
        request_payload = self._new_request(0)
        request_payload.append_string(dpname) # dpname
        request_payload.append_string("SUBSCRIBE")

        # send request
        self.send_packet(0x4f0b, request_payload)

        # receive response (#1)
        (protocol, response_payload) = self.recv_packet()

        # handle response (#1)

        assert protocol == 0x8f0c

        response_seqnr        = response_payload.peel_number(8) # seqnr
        response_replynr      = response_payload.peel_number(8) # replynr
        response_peertime_ns  = response_payload.peel_timeval2ns()
        response_response     = response_payload.peel_string()
        response_dptype       = response_payload.peel_string()

        print "-- seqnr          ", response_seqnr
        print "-- replynr        ", response_replynr
        print "-- peertime_ns    ", ns2dt(response_peertime_ns)
        print "-- response       ", response_response
        print "-- dptype         ", response_dptype

        # receive response (#2)
        while True:
            (protocol, response_payload) = self.recv_packet()
            # interpret response

            assert protocol == 0x8f0d

            response_seqnr        = response_payload.peel_number(8) # seqnr
            response_replynr      = response_payload.peel_number(8) # replynr
            response_peertime_ns  = response_payload.peel_timeval2ns()
            response_payload_ns   = response_payload.peel_timeval2ns()
            response_value        = response_payload.peel_string()

            assert len(response_payload) == 0

            print "-- seqnr          ", response_seqnr
            print "-- replynr        ", response_replynr
            print "-- peertime_ns    ", ns2dt(response_peertime_ns)
            print "-- payload_ns     ", ns2dt(response_payload_ns)
            print "-- value          ", response_value
            sys.stdout.flush()
