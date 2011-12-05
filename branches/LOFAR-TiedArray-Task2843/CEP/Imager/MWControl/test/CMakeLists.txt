# $Id$

include(LofarCTest)

lofar_add_test(tMWControl
  tMWControl.cc PredifferTest.cc SolverTest.cc MWStepTester.cc)
lofar_add_test(tMWSpec tMWSpec.cc)
