import selfcal.selfcal;
import selfcal.paramset;
ass = selfcal.paramset.list2JobAssignment([False,True])
par = selfcal.paramset.list2ParamSet([12.345,67.890])
eng = selfcal.selfcal.SelfcalEngineStub()
eng.init(2,par["data"])
eng.Solve(ass["data"],par["data"])
