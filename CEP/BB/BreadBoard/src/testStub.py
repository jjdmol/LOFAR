#! /usr/bin/env python

##############################
#include "SelfcalEngineStub.h"
from selfcal import selfcal;

#include <stdlib.h>
import whrandom
Generator = whrandom.whrandom();
#include <iostream.h>

##############################
#main () {
def main():
  #SelfcalEngineStub myStub;
  myStub = selfcal.SelfcalEngineStub();
  
  #float *myparams=new float[8];
#  myparams = [selfcal.new_floatp(),selfcal.new_floatp(),selfcal.new_floatp(),selfcal.new_floatp(),selfcal.new_floatp(),selfcal.new_floatp(),selfcal.new_floatp(),selfcal.new_floatp()];
  myparams = selfcal.pars(8);

  def getRandom():
    return (100.0*Generator.random()) +1.0;

  #cout << "parameters : ";
  print "parameters : " ,;

  #for (int i=0; i<8; i++) {
  for i in (0, 1, 2, 3, 4, 5, 6, 7):
    print myparams[i];
    #// fill with random pattern of numbers [0..100]
    #myparams[i] = (100.0*random() / RAND_MAX) +1.0;
    myparams.__setitem__(i, getRandom());
    #cout << myparams[i] << " ";
    print myparams[i], " " ,;
    #}
  #cout << endl;
  print;
  #next line is c and python
  myStub.init(8, myparams);
  
  #// fill the work definition array
  #bool *workdef=new bool[8];
  workdef = selfcal.bools(8);
  #cout << "workdef    :" ;
  print "workdef    :" ,;
  #for (int i=0; i<8; i++) {
  for i in (0, 1, 2, 3, 4, 5, 6, 7):
    #int rand100 = (int)(100.*random()/RAND_MAX+1.0);
    rand100 = (100.*Generator.random() + 1.0);
    #next line is c and python
    workdef.__setitem__(i,(rand100 > 80 ));
    #cout << workdef[i] << " ";
    print workdef[i], " " ,;
  #}
  #cout << endl;
  print;

#  wr = selfcal.new_ppars();
#  selfcal.ppars_assign(wr,myparams);

  #for (int i=0; i<15; i++) {
  for i in (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14):
    #myStub.Solve(workdef, &myparams);
    myparams = myStub.Solve(workdef, myparams);
    #next line is c and python
    myStub.dump();
  #}
  del myparams;
  del myStub;
  print "end of main";  
#} end of main

main();

