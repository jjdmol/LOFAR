#!/usr/bin/python

import sys
from lofar.messaging.RPC import RPC

''' Simple RPC client for Service momqueryservice.GetProjectDetails
'''

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print 'Please provide one or more mom ids'
        sys.exit(1)

    ids = ','.join(sys.argv[1:])
    with RPC('momqueryservice','GetProjectDetails',timeout=10) as getProjectDetails:
        res,status=getProjectDetails(ids)

        if status == 'OK':
            for id,obj in res.items():
                print 'Object %s' % (id)
                for k,v in obj.items():
                    print '  %s: %s' % (k, v)
                print
        else:
            print status, res

