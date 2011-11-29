import unittest
import xmlrunner

class AutoTester(unittest.TestCase):
    def __init__(self):
        print '__init__'
        pass

    def setUp(self):
        print '__setUp'

    def test_keys(self):
        self.assertEqual(True)

def suite():

    suite = unittest.TestSuite()

    suite.addTest(AutoTester)

    return suite
 
            
if __name__ == "__main__":    
    testRunner=xmlrunner.XMLTestRunner(output='test-reports.xml').run(suite())
    #unittest.main()