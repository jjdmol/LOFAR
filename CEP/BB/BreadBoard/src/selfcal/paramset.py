#!/usr/bin/env python

"""
some util funcs for selfcal engines
"""

###
#
# The folowing could be isolated from the selfcal package. These are
# just parameter types.
#
import selfcal;

def list2ParamSet(lst):
  """ """
  num = len(lst);
  myparams = {};
  myparams["length"] = num;
  myparams["data"] = selfcal.pars(num);
  i = 0;
  while ( i < num ):
    #// fill with random pattern of numbers [0..100]
    myparams["data"].__setitem__(i, lst[i]);
    i = i+1;
  return myparams;

def ParamSet2list(paramset):
  lst = [];
  num = paramset["length"];
  i = 0;
  while (i < num):
    lst.append(paramset["data"].__getitem__(i));
    i = i + 1;
  return lst;

def list2JobAssignment(lst):
  """ """
  num = len(lst);
  workdef = {};
  workdef["length"] = num
  workdef["data"] = selfcal.bools(num);
  i = 0
  while( i < num):
    workdef["data"].__setitem__(i,lst[i]);
    i = i+1;
  return workdef;

def JobAssignment2list(jobassignment):
  lst = [];
  num=jobassignment["length"];
  i = 0;
  while (i < num):
    lst.append(jobassignment.__getitem__(i));
    i = i + 1;
  return lst;

