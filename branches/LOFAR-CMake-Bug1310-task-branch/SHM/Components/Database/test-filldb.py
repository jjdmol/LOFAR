#! /usr/bin/python2.4

# This is a test script that stores random data into the database.

import os, random, datetime

def queries():

    stations = []
    for i in range(32):
        stations.append("CS%02d" % (i+1))
    for i in range(45):
        stations.append("RS%02d" % (i+1))

    while True:

        si_name = stations[random.randint(0, len(stations)-1)]
        crate_id = random.randint(0, 5)
        rsp_id   = random.randint(0, 3)
        ap_id    = random.randint(0, 3)
        rcu_id   = random.randint(0, 1)
        obs_name = 'crate%d_rsp%d_ap%d_rcu%d_MeanPower' % (crate_id, rsp_id, ap_id, rcu_id)
        value    = random.random()

        now      = datetime.datetime.now()    
        time     = "%04d-%02d-%02d %02d:%02d:%02d.%06d" % (now.year, now.month, now.day, now.hour, now.minute, now.second, now.microsecond)

        yield "SELECT InsertObservation(TIMESTAMP WITH TIME ZONE '%(time)s', '%(si_name)s', '%(obs_name)s', '%(value)s');" % vars();

def main():
    for query in queries():
        print query

if __name__ == "__main__":
    main()
