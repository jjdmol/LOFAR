#!/usr/bin/env python

"""
The pgdate.py class

$Id$

"""

import datetime;
import string;

def string2date(s):
  components = string.split(s,' ');
  datepart = components[0];
  timepart = components[1];
  dateparts = string.split(datepart,'-');
  timeparts = string.split(timepart,':');
  year = string.atoi(dateparts[0]);
  month = string.atoi(dateparts[1]);
  day = string.atoi(dateparts[2]);
  hour = string.atoi(timeparts[0]);
  minute = string.atoi(timeparts[1]);
  seconds = string.split(timeparts[2],'.');
  second = string.atoi(seconds[0]);
  microsecond = string.atoi(seconds[1]);
  return datetime.datetime(year, month, day, hour, minute, second, microsecond);
