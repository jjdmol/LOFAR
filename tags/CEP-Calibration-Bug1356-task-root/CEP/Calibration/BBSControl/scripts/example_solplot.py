import lofar.parmdb
import solplot

db = lofar.parmdb.parmdb("instrument.db")

stations = ["CS00%d_HBA%d" % (x,y) for x in [1,8] for y in range(0,4)]
(ampl, phase) = solplot.fetch(db, stations)

solplot.plot(ampl)
