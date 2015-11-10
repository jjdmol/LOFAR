#!/usr/bin/python

import lofar.messaging.RPC as RPC

with RPC("GetServerState",busname="SystemStats") as myrpc:
    for i in range(2):
        res,status=myrpc("CEP4")
        for node,info in res['nodes'].iteritems():
          """
          Service returns:
		'hostid': int
        	'totalspace': int
        	'hostname': unicode
        	'usedspace': int
        	'id': int
        	'statusid': int
        	'claimedspace': int
        	'path': unicode
        	'groupid': int
          """
          print("-------------")
          print(" Node: %-10s Status: %s" %(node,str(info['status'])))
          storage=info['storage']
	  for store in info['storage']:
		print("    -  Storage: %-12s total: %8d used: %8d claimed: %8d" %
                     (store['path'],store['totalspace'],store['usedspace'],
                      store['claimedspace']))


