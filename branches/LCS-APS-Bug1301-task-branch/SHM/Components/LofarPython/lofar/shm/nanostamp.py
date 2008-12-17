
# The number of nanoseconds between 1970-01-01 and 2000-01-01 is
# 946684800000000000

import time, datetime, calendar

def now2ns():
    return dt2ns(datetime.datetime.utcnow())
    
def dt2ns(dt = None):
    """Translate a 'datetime' style tuple to nanoseconds-since-1970"""
    sec = calendar.timegm((dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second))
    nsec = 1000 * dt.microsecond
    return sec * 1000000000 + nsec

def ns2dt(ns):
    (sec, nsec) = (ns//1000000000, ns%1000000000)

    s = time.gmtime(sec)
    usec = nsec//1000

    return datetime.datetime(
        s.tm_year, s.tm_mon, s.tm_mday,
        s.tm_hour, s.tm_min, s.tm_sec, usec)
