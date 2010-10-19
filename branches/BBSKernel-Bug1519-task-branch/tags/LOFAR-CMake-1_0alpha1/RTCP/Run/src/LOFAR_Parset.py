import os
import time
import string
import copy
import scanf

class switch(object):
    def __init__(self, value):
        self.value = value
        self.fall = False

    def __iter__(self):
        """Return the match method once, then stop"""
        yield self.match
        raise StopIteration
    
    def match(self, *args):
        """Indicate whether or not to enter a case suite"""
        if self.fall or not args:
            return True
        elif self.value in args: # changed for v1.5, see below
            self.fall = True
            return True
        else:
            return False

class Parset(switch):

    def __init__(self, defaults = dict()):
        self.parameters = defaults

    def readFromFile(self, fileName):
        lastline = ''
        for line in open(fileName, 'r').readlines():
            lastline = lastline + line.split('#')[0]
            lastline = lastline.rstrip()
            if len(lastline) > 0 and lastline[-1] == '\\':
                lastline = lastline[:-1]
            elif '=' in lastline:
                key, value = lastline.split('=')
                self.parameters[key.strip()] = value.strip()
                lastline = ''

        # Add keys from file: OLAP.parset.
        lastline = ''
        for line in open('OLAP.parset', 'r').readlines():
            lastline = lastline + line.split('#')[0]
            lastline = lastline.rstrip()
            if len(lastline) > 0 and lastline[-1] == '\\':
                lastline = lastline[:-1]
            elif '=' in lastline:
                key, value = lastline.split('=')
                self.parameters[key.strip()] = value.strip()
                lastline = ''
		
    def find_first_of(self, key):
	digit = False
	for dig in range(0,10):
	    if (key[0] == str(dig)):
	        digit = True
	        break;
	if not digit:
	        print key + ' is not an digit'
		sys.exit(0)
	
    def stripBraces(self, orgStr):
        baseString = copy.deepcopy(orgStr)
        baseString = baseString.strip('[').rstrip(']')
	return baseString

    def findRangeElement(self, orgStr):
        if orgStr.find('(') != -1 : return "("
	if orgStr.find('*') != -1 : return "*"
	if orgStr.find('..') != -1 : return ".."
        return orgStr   
    
    def expandedArray(self, orgStr):
 	if orgStr.find('..') == -1 and orgStr.find('*') == -1:
	    return orgStr    #no, just return original

        baseString = self.stripBraces(orgStr)
        # and split into a vector
        strVector = string.split(baseString, ',')
        # note: we assume that the format of each element is [xxxx]9999
        self.find_first_of(strVector[0]) 
        # construct scanmask and outputmask.
	scanMask = "%05d"
	outMask = '%01ld'
        
	# handle all elements
	result = '['
	nrElems = len(strVector);
	for idx in range(0, nrElems):
	    firstVal = scanf.sscanf(strVector[idx],scanMask)[0]
 	    for case in switch(self.findRangeElement(strVector[idx])):
                if case('('):
		    rangePos = strVector[idx].find('(')
		    grpStr = strVector[idx][rangePos+1:len(strVector[idx])-1]
		    grpStr = grpStr.replace(";", ",")
                    break
                if case('*'):
		    rangePos = strVector[idx].find('*')
		    lastVal = strVector[idx][rangePos+1:len(strVector[idx])]
                    break
                if case('..'):
		    rangePos = strVector[idx].find('..')
		    lastVal = strVector[idx][rangePos+2:len(strVector[idx])]
		    
		    # check range
		    if lastVal < firstVal:
		        print 'Illegal range specified in ' + strVector[idx] + '. Returning orignal string'
	                return orgStr
                    break
                if case():
		    lastVal = firstVal;
                    # No need to break here, it'll stop anyway	    
	    
	    # finally construct one or more elements
	    firstElem  = True
	    for case in switch(self.findRangeElement(strVector[idx])):
                if case('('):
		    for val in range(0, int(firstVal)):
			if firstElem:
			    eStr = self.stripBraces(self.expandedArray(grpStr))
			    result += eStr
		            firstElem = False
		        else:
		            result += ',' + eStr
  		    break
                if case('*'):
		    for val in range(0, int(firstVal)):
			if firstElem:
			    result += str(lastVal)
			    firstElem = False
		        else:
		            result += ',' + str(lastVal) 
 		    break
                if case('..'):
		    tmp = False
                if case():
		    for val in range(int(firstVal), int(lastVal)+1):
			if firstElem:
			    result += str(val)
		            firstElem = False
		        else:
		            result += ',' + str(val)
	    if idx != len(strVector)-1:
	        result += ','		    
	result += ']'
	return result
	
    def writeToFile(self, fileName):
        outf = open(fileName, 'w')
        for key, value in sorted(self.parameters.iteritems()):
            outf.write(key + ' = ' + str(value) + '\n')
        outf.close()

    def getString(self, key):
        return self.parameters[key]

    def getInt32(self, key):
        return int(self.parameters[key])

    def getFloat(self, key):
        return float(self.parameters[key])

    def getStringVector(self, key):
        line = str(self.parameters[key])
	if line == '[]' : return list()
	line = string.replace(line,'\'','')
        line = line.strip('[').rstrip(']')
	line = string.replace(line,' ','')
        return line.split(',')

    def getExpandedInt32Vector(self, key):
        ln = self.expandedArray(self.parameters[key])
	ln = ln.strip('[').rstrip(']')
	return [int(lp) for lp in ln.split(',')]
	
    def getInt32Vector(self, key):
        ln = str(self.parameters[key])
	ln = ln.strip('[').rstrip(']')
  	ln = string.replace(ln,' ','')
        return [int(lp) for lp in ln.split(',')]
	
    def getFloatVector(self, key):
        line = self.parameters[key]
        line.strip('[').strip(']')
        return [float(lp) for lp in line.split(',')]

    def getBool(self, key):
        return self.parameters[key] == 'T'
   
    def isDefined(self, key):
        return key in self.parameters

    def __contains__(self, key):
        return key in self.parameters
        
    def __setitem__(self, key, value):
        self.parameters[key] = value

    def __getitem__(self, key):
        return self.parameters[key]



