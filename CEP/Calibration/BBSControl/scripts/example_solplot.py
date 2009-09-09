import lofar.parmdb
import lofar.solutions

db = lofar.parmdb.parmdb("instrument.db")

stations = ["CS00%d_HBA%d" % (x,y) for x in [1,8] for y in range(0,4)]
(ampl, phase) = lofar.solutions.fetch(db, stations)

lofar.solutions.plot(ampl)
