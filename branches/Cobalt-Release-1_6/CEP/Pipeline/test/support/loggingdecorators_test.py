from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile
import xml.dom.minidom as xml

from lofarpipe.support.loggingdecorators import xml_node, duration
from lofarpipe.support.xmllogging import get_child, get_active_stack
#imports from fixture:


class loggingdecoratorsTest(unittest.TestCase):
    def __init__(self, arg):
        super(loggingdecoratorsTest, self).__init__(arg)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_xml_node_single_depth_timing_logging(self):
        """
        Test single nested duration logging
        Output xml is compared as is.
        """
        class Test(object):
            @xml_node
            def test(self):
                pass

        an_object = Test()
        an_object.test()

        #calling a decorated function should result in a active_stack node on the 
        # class. After finishing it should have the duration in there
        target_xml = '<active_stack Name="Test" type="active_stack"><active_stack/><test duration="0.0"/></active_stack>'

        self.assertTrue(float(get_child(
            an_object.active_stack, "test").getAttribute("duration")) <= 0.1,
            "The created active stack did not add the duration information")

    def test_xml_node_nested_timing_logging(self):
        """
        Test nested logging. The duration is variable. Test existance of 
        duration attribute and test that size of the created xml log is small.
        """
        class Test(object):
            @xml_node
            def test(self):
                pass

            @xml_node
            def test2(self):
                self.test()

        an_object = Test()
        an_object.test2()

        #calling a decorated function should result in a active_stack node on the 
        # class. After finishing it should have the duration in there
        target_xml = '<active_stack Name="Test" type="active_stack"><active_stack/><test duration="0.0"/></active_stack>'
        child2 = get_child(an_object.active_stack, "test2")
        child1 = get_child(child2, "test")
        self.assertTrue(float(child1.getAttribute("duration")) < 0.1,
                        "The duration was to large for the size of the test function")
        self.assertTrue(float(child2.getAttribute("duration")) < 0.1,
                        "The duration was to large for the size of the test function")

    def test_xml_node_return_value(self):
        """
        assure that the return value of the decorated function is still correct
        """
        class Test(object):
            @xml_node
            def test(self):
                return "a value"

        an_object = Test()
        return_value = an_object.test()

        self.assertTrue(return_value == "a value" ,
                        "The decorated function did not return the actual function return value ")

    def test_duration_context_manager(self):
        """
        Test that on entering the context with self the containing object
        pointer is added. It should also continue to exist after leaving the
        context
        
        """
        class tester(object):
            def __init__(self):
                pass

            def test(self):
                if get_active_stack(tester) is not None:
                    print "An active stack should only exist when added explicitly"
                    return False

                with duration(self, "a name") as context_object:
                    active_stack = get_active_stack(self)
                    # We should have an active stack in the context
                    if active_stack is None:
                        print "In duration context the active stack should be added."
                        return False

                    if not get_child(
                        active_stack, "active_stack").hasChildNodes():
                        print "in the context the active_stack should at least contain one entry"
                        return False
                    # Now leave the  context

                if get_child(
                        active_stack, "active_stack").hasChildNodes():
                        print "After the context the active stack should be left"
                        # There is stil an entry in the active stack
                        return False

                return True

        test_object = tester()
        self.assertTrue(test_object.test(), "The duration context returned with False")

