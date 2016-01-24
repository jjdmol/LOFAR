from lofar.parmdb import *

def showValues (pdb, pattern='*', nf=4, nt=2):
    # Get the names.
    print pdb.getNames()
    # Get the range.
    rng = pdb.getRange()
    print rng
    # Get the values.
    print pdb.getValuesStep(pattern, rng[0], rng[2], nf, rng[1], rng[3], nt, True)
    # Get values and grid.
    print pdb.getValuesGrid(pattern, rng[0], rng[2], rng[1], rng[3])
    # Print default names and values.
    print pdb.getDefNames(pattern);
    print pdb.getDefValues(pattern)

# The test is the same as in tParmFacade.cc.
# Open the parameterset (created in .run file).
pdb=parmdb("tpyparmdb_tmp.pdb")
print ">>>"
print pdb.version("tree")
print pdb.version("full")
print pdb.version("top")
print pdb.version()
print "<<<"
showValues (pdb);
showValues (pdb, '', 1);
showValues (pdb, 'parm1', 1);
