import os
import unittest
import getopt
import string
import inspect
import sys
import re

class Discover():
    """
    Discover class collects all unit test case in <path> and recursive directories
    Collects them in a single large suite.
    Start at supplied <path> an add all tests in files matching the supplied expression and
    all individual tests matching the expression 
    """
    #TODO: ordering of suites is not controlled atm.   
    suite = unittest.TestSuite()
    
    def __init__(self,path,pattern):
        
        #match with all (used for a filename matches with expression: all individual test must be loaded
        allMatcher = re.compile(".*")
        #matcher for the expression
        patternMatcher = re.compile(pattern)
        
        for root, dirs, files in os.walk(path):            
            dirSuite = unittest.TestSuite()
            for name in files:                   
                fileNameParts = name.split('.')
                #assert correct file extention 
                if (len(fileNameParts) == 1) or (fileNameParts[1] !=  'py'):
                    continue
                #try loading as a module
                try: 
                    module = __import__(fileNameParts[0] , globals={}, locals={})
                except:
                    continue                   
                    
                #the expression mechanism
                testMatcher = None
                if patternMatcher.match(name):
                    testMatcher = allMatcher      #if current dir matches with expression include all tests
                else:
                    testMatcher = patternMatcher                                
                #create a test suite
                fileSuite = unittest.TestSuite()
                testnames = dir(module)

                #add all cases ending with test and match the regexp search string
                for testName in testnames:
                    if testName.endswith('Test') or testName.endswith('test'):
                        try:
                            testClass = getattr(module, testName) #try loading it
                            if inspect.isclass(testClass):    #if class add it
                                if not testMatcher.match(testName):  #if not matching with 
                                    continue 
                                fileSuite.addTest(unittest.makeSuite(testClass))
                        except:
                            pass
                        
                #if tests found add the file suite to the directory suite
                if fileSuite.countTestCases() != 0:
                    dirSuite.addTest(fileSuite)
                    
            #add to top level suite
            if dirSuite.countTestCases() != 0:
                self.suite.addTest(dirSuite)

class UnitTesterTest(unittest.TestCase):
    """
    Self test for the UnitTester
    """
    #TODO: Add propper test suite, creating come files and try laoding it (multiple directories and depths)
    def setUp(self):
        self.tester = "A test string"

    def test_validator(self):
        """
        Check that current testClass is loaded and performed
        """
        self.assertTrue(self.tester == "A test string")


def usage():
    """
    Display a short overview of available arguments
    """
    usage = r"""Usage: python UnitTester [-p <path = '.'> -e <expression = *>  -h -x]
    Recursively look in path for unit test classes matching expression.
    Collect in a single suite and run them
    -p, --path <path> to start looking. Default is '.'
      -e, --exp  <expresion> to match with found classes to perform a subset of the tests
      or
      -m, --matchword match with found classes to perform a subset of tests (shorthand for .*arg.* expression
    -h, --help Display this usage
    -x, --xml  <filename> Export resuls to xml (results are overwritten) 
    """
    print usage

if __name__ == "__main__":

    #Default parameters settings
    path = '.'
    expression = '.*'
    xml = ""
    
    #parse command lines and set parameters for Discover function
    try:         
        opts, args = getopt.getopt(sys.argv[1:], "p:e:hx:m:", ["path=", "exp=", "help", "xml=","matchword="])                       
    except getopt.GetoptError:          
        usage()                         
        sys.exit(2)  
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        elif opt in ( "-p", "--path"):
            path = arg
        elif opt in ("-e", "--exp"):
            expression = arg
        elif opt in ("-x", "--xml"):
            xml =  arg
        elif opt in ("-m","--matchword"):
            expression = ".*{0}.*".format(arg)

    #Collect tests from files and paths    
    test = Discover(path, expression)
    
    #deside on unit testrunner to use
    if xml:
        import xmlrunner
        testRunner=xmlrunner.XMLTestRunner(output=xml).run(test.suite)
    else:
        unittest.TextTestRunner(verbosity=2).run(test.suite)
