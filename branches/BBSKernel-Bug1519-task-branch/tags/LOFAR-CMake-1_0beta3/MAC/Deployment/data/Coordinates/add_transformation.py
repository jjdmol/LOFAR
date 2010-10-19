#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb,pg

def getInputWithDefault(prompt, defaultValue):
    answer = defaultValue
    answer = raw_input(prompt+" ["+str(defaultValue)+"]: ").upper()
    if (len(answer)<=1): answer=defaultValue
    return answer

#
# MAIN
#
if __name__ == '__main__':    

    # check syntax of invocation
    # Expected syntax: add_transformation
    #
    if (len(sys.argv) != 1):
        print "Syntax: %s" % sys.argv[0]
        sys.exit(1)

    from_frame = getInputWithDefault("Referenceframe to convert from","ETRF89")
    to_frame   = getInputWithDefault("Referenceframe to convert to","ITRF2005")
    target_date= getInputWithDefault("Targetdate (yyyy.yy)",2009.0)
    Tx         = getInputWithDefault("Tx (cm)     ",0.0)
    Ty         = getInputWithDefault("Ty (cm)     ",0.0)
    Tz         = getInputWithDefault("Tz (cm)     ",0.0)
    sf         = getInputWithDefault("sf (10e-9)  ",0.0)
    Rx         = getInputWithDefault("Rx (0.001'')",0.0)
    Ry         = getInputWithDefault("Ry (0.001'')",0.0)
    Rz         = getInputWithDefault("Rz (0.001'')",0.0)

# ... to be continued
