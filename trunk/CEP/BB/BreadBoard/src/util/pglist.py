#!/usr/bin/env python

"""
The pgdate.py class

$Id$

"""

import datetime;
import string;

debug = None;

def pgArray2list(s):
  """ not done yet """
  lst = [];
  """ s is a string comming in.
  it should not contain { and } as outermost non-whitespace chars
  """
  """next is a bracket, a field terminated with a comma or whitespace """
  s = string.strip(s);
  if(debug):
    print "start string: " + s;
##  s = s[1:len(s)];
  (nextfield , commapos) = getField(s);
  lst.append(nextfield);
  while(commapos != -1):
    s = s[(commapos + 1):];
    rc = getField(s);
    if(rc):
      (nextfield , commapos) = rc;
    else:
      commapos = -1;
    lst.append(nextfield);
  return lst;

def getField(s):
  if(debug):
    print "getting field from: " + s;
  s = string.strip(s);
  rc = None;
  if(len(s) == 0):
    rc = (s, -1);
  elif(s[0] == "{"):
    right = find_matching_bracket(s);
    next = string.find(s,",",right);
    rc =  (pgArray2list(s[1:right]), next);
  else:
    next = string.find(s,",");
    if(next > 0):
      s = s[:next];
    """ check for a number """
    try:
      s = int(s)
    except:
      """do nothing"""
    rc =  (s, next);
  if(debug):
    print rc;
  return rc;

def find_matching_bracket(s):
  """  """
  if(debug):
    print "matching in: " + s;
  if(s[0] != "{"):
    return -1;
  left = string.find(s,"{",1);
  right = string.find(s,"}",1);
  if(-1 < left < right):
    left = left + find_matching_bracket(s[left:]);
    right = string.find(s,"}",left + 1);
    if(debug):
      print "left,right: (" + str(left) + "," + str(right) + ", matched string: " + s[1:right];
  return right;

def list2pgArray(l):
  """ not done yet """
  if(isList(l)):
    tmplst = [];
    for elem in l:
      tmplst.append(list2pgArray(elem));
    strrep = "{" + string.join(tmplst,", ") + "}";
  else:
    strrep = str(l);
  return strrep;

def isList(v):
  try:
    len(v);
  except TypeError:
    return 0;
  else:
    return 1;

def justThePgArray(s):
  """this function is to simple if s contains a sequence of pgArrays:
  "{ waetfg }, {erfg}"
  """
  leftside = string.find(s,"{");
  rightside = find_matching_bracket(s);
  s = s[leftside:rightside+1];
  return s;

