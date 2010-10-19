#! /usr/bin/python2.4

import sys, os, cgi, lofar.shm.db, lofar.shm.sbstats

def main():

    qs = cgi.parse_qs(os.environ["QUERY_STRING"])

    si_name    = qs.get("si_name", ["FTS-1"])[0]
    rcu_id     = qs.get("rcu_id" , [0]      )[0]
    width      = qs.get("width"  , [640]    )[0]
    height     = qs.get("height" , [480]    )[0]

    rcu_id = int(rcu_id)
    width = int(width)
    height = int(height)

    shmdb = lofar.shm.db.SysHealthDatabase()
    shmdb.open()

    sbstats = lofar.shm.sbstats.SubbandStatistics()
    valid = sbstats.getFromDB(shmdb, si_name, rcu_id, None)
    shmdb.close()

    if valid:
    
        sys.stdout.write("Content-type: image/png\n\n")

	png_string = sbstats.gnuplot((width, height))
        sys.stdout.write(png_string)

if __name__ == "__main__":
    main()
