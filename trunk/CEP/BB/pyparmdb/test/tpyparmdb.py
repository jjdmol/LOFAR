from lofar.parmdb import *

def showValues (pattern='*', nf=4, nt=2):
    # Get the names.
    print pdb.getNames()
    # Get the range.
    rng = pdb.getRange()
    print rng
    # Get the values.
    print pdb.getValues(pattern, rng[0], rng[2], nf, rng[1], rng[3], nt, True)
    # Get values and grid.
    print pdb.getValuesGrid(pattern, rng[0], rng[2], rng[1], rng[3])

# The test is the same as in tParmFacade.cc.
# Open the parameterset (created in .run file).
pdb=parmdb("tpyparmdb_tmp.pdb")
showValues ();
showValues ('', 1);
