#!/usr/bin/env python


"""
$Id$

some util funcs for selfcal engines

"""

# make a number generator
import whrandom
Generator = whrandom.whrandom();
def getRandom():
  return (100.0*Generator.random()) +1.0;

def unsetRandom():
  Generator.seed(1,1,1);

def mkParamSet(num):
  """ """
  myparams = [];
  print "parameters : " ,;
  i = 0;
  while( i < num):
    #// fill with random pattern of numbers [0..100]
    myparams.append(getRandom());
    print myparams[i], " " ,;
    i = i+1;
  print;
  return myparams;


def mkJobAssignment(num):
  """ """
  workdef = []
  print "workdef    :" ,;
  i = 0
  while( i < num):
    rand100 = getRandom();
    workdef.append(getRandom() > 80 );
    print workdef[i], " " ,;
    i = i+1;
  print;
  return workdef;

