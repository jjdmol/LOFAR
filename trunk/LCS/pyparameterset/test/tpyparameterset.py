from lofar.parameterset import *

def checkps (ps):
    print ps.isDefined("key1")
    print ps.isDefined("a.b")
    print ps.get("a.b").get()
    print ps.getString("a.b")
    print ps.getString("a.b", "aa")
    print ps.getString("aa.bb", "aa")

    print ps.getString("a.b.lange_naam")

    print ps.getBool(key="a.b.bool")
    print ps.getBool("a.b.bool", False)
    print ps.getBool("aa.bb", False)

    print ps.getInt("a.b")
    print ps.getInt("a.b", 10)
    print ps.getInt("aa.bb", 10)

    print ps.getFloat("a.b")
    print ps.getFloat("a.b", 3.14)
    print ps.getFloat("aa.bb", 3.14)
    print ps.getDouble("a.b.double")

    print ps.getBoolVector("vecbool")
    print ps.getBoolVector("vecbool", (False,True))
    print ps.getBoolVector("aa.bb", [False,True])

    print ps.getIntVector("vec")
    print ps.getIntVector("vec", (5,6))
    print ps.getIntVector("aa.bb", [5,6])

    print ps.getFloatVector("vec")
    print ps.getFloatVector("vec", (5,6))
    print ps.getFloatVector("aa.bb", [5,6])

    print ps.getDoubleVector("vec")
    print ps.getDoubleVector("vec", (5,6))
    print ps.getDoubleVector("aa.bb", [5,6])

    print ps.getStringVector("vec")
    print ps.getStringVector("vec", ('5','6'))
    print ps.getStringVector("aa.bb", ['5','6'])

    print ps.getIntVector("vecexp", True)
    print ps.getIntVector("vecexp", [1,2], True)
    print ps.getIntVector("aa.bb", [1,2], True)

    pvs = ps["vecnest"]
    print pvs.isVector()
    pvsvec = pvs.getVector()
    print pvsvec[0].get()
    print pvsvec[0].expand().getIntVector()
    print pvsvec[1].expand().getIntVector()

# Check using given parset file.
checkps (parameterset("tpyparameterset.in"))
print ""

# Create and check a new parset using same keys/values as in parset file.
ps=parameterset()
print ">>>"
print ps.version("tree")
print ps.version("full")
print ps.version("top")
print ps.version()
print "<<<"
ps.add ("a.b", "7")
ps.add ("a.b.lange_naam", "dit is nu een andere naam geworden met extra spaties aan het einde  ")
ps.add ("a.b.c", "5")
ps.add ("a.b.double", "3.1415926")
ps.add ("a.b.bool", "true")
ps.add ("e.g", "een voorbeeld")
ps.add ("egg", "een ei")
ps.add ("vecbool", "[true,false,true]")
ps.add ("vec", "[1,2,3]")
ps.add ("vecexp", "[1..3,5..10]")
ps.add ("vecnest", "[[1..3,5*10],[5..10]]")
checkps (ps)
