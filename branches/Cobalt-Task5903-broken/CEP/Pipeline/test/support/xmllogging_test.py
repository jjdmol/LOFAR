from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile
import xml.dom.minidom as xml

import lofarpipe.support.xmllogging as xmllogging
from lofarpipe.support.xmllogging import get_child

#imports from fixture:


class xmlloggingTest(unittest.TestCase):
    def __init__(self, arg):
        super(xmlloggingTest, self).__init__(arg)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_add_child(self):
        local_document = xml.Document()
        head = local_document.createElement("head")
        returned_node = xmllogging.add_child(head, "child")


        self.assertTrue(len(head.childNodes) == 1,
                         "add_child add more then one child")
        self.assertTrue(head.childNodes[0].tagName == "child",
                        "add_child added a child with an incorrect name")
        self.assertTrue(returned_node == head.childNodes[0],
                        "add_child should return the created node")


    def test_get_child(self):
        local_document = xml.Document()
        head = local_document.createElement("head")
        child = xmllogging.add_child(head, "child")
        second_child = xmllogging.add_child(head, "second_child")
        third_child = xmllogging.add_child(head, "child")

        # call the function
        returned_child = get_child(head, "child")

        # test output
        self.assertTrue(returned_child == child,
                "get_child dit not return the first child matching the name")
        self.assertTrue(returned_child != third_child,
                "get_child dit not return the first child matching the name")

    def test_get_child_not_found(self):
        local_document = xml.Document()
        head = local_document.createElement("head")
        child = xmllogging.add_child(head, "child")

        # call the function
        returned_child = get_child(head, "does_not_exist")

        # test output
        self.assertTrue(returned_child == None,
                "when no children are found get_child should return None")


    def test_enter_active_stack(self):
        class a_class(object):
            def __init__(self):
                pass


        an_object = a_class()

        xmllogging.enter_active_stack(an_object, "test")

        # The name of the create xml node should be active_stack
        self.assertTrue(an_object.active_stack.tagName == "active_stack")

        # The created node should have child active_stack with the actual active
        # node
        active_stack_node = xmllogging.get_child(
                an_object.active_stack, "active_stack")
        test_node = xmllogging.get_child(
                active_stack_node, "test")
        self.assertTrue(test_node.tagName == "test")

        xmllogging.exit_active_stack(an_object)

        # Test if after leaving the stack the test node is moved to root and
        # that that the active node is empty:
        xml_target_output = '<active_stack Name="a_class" type="active_stack"><active_stack info="Contains functions not left with a return"/><test/></active_stack>'
        #self.assertTrue(an_object.active_stack.toxml() ==
        #               pretty_xml_target_output)
        self.assertTrue(xml_target_output == an_object.active_stack.toxml(),
                        an_object.active_stack.toxml())


    def test_enter_active_stack_twice(self):
        class a_class(object):
            def __init__(self):
                pass


        an_object = a_class()

        xmllogging.enter_active_stack(an_object, "test")
        xmllogging.enter_active_stack(an_object, "test2")
        # The name of the create xml node should be active_stack
        self.assertTrue(an_object.active_stack.tagName == "active_stack")

        # The created node should have child active_stack with the actual active
        # node
        active_stack_node = xmllogging.get_child(
                an_object.active_stack, "active_stack")
        test_node = xmllogging.get_child(
                active_stack_node, "test")
        self.assertTrue(test_node.tagName == "test")

        # and a second node with the name test2
        test_node2 = xmllogging.get_child(
                    active_stack_node, "test2")
        self.assertTrue(test_node2.tagName == "test2")

        xmllogging.exit_active_stack(an_object)
        xmllogging.exit_active_stack(an_object)

        # after leaving the stack completely there should be a nested 
        # node structure with the two test nodes 
        xml_target_output = '<active_stack Name="a_class" type="active_stack"><active_stack info="Contains functions not left with a return"/><test><test2/></test></active_stack>'
        #self.assertTrue(an_object.active_stack.toxml() ==
        #               pretty_xml_target_output)
        self.assertTrue(xml_target_output == an_object.active_stack.toxml(),
                        an_object.active_stack.toxml())

    def test_exit_incorrect_active_stack(self):
        class a_class(object):
            def __init__(self):
                pass

        an_object = a_class()
        # Raise ValueError on leaving non existing stack
        self.assertRaises(ValueError, xmllogging.exit_active_stack, an_object)

    def test_get_active_stack(self):
        class a_class(object):
            def __init__(self):
                pass


        an_object = a_class()
        result = xmllogging.get_active_stack(an_object)

        # If no active stack is created return None
        self.assertTrue(result == None, "When no active stack is entered"
                        " get_active_stack should return None")

        xmllogging.enter_active_stack(an_object, "test", stack_name="test_stack")

        result = xmllogging.get_active_stack(an_object)
        # Calling get stack with incorrect name (default in this case) return None
        self.assertTrue(result == None, "When incorrect active stack name is entered"
                        " get_active_stack should return None")


        xmllogging.exit_active_stack(an_object, stack_name="test_stack")


    def test_add_child_to_active_stack_head(self):
        class a_class(object):
            def __init__(self):
                pass

        local_document = xml.Document()
        created_node = local_document.createElement("Tester")

        an_object = a_class()
        return_value = xmllogging.add_child_to_active_stack_head(an_object,
                        created_node)

        self.assertTrue(return_value == None,
            "function should return None when adding child when no active stack is there ")


        xmllogging.enter_active_stack(an_object, "test")
        # Add the chilf
        return_value = xmllogging.add_child_to_active_stack_head(an_object,
                                                           created_node)
        # get the stack
        stack = xmllogging.get_active_stack(an_object)
        stack_text = stack.toxml()
        goal_text = """<active_stack Name="a_class" type="active_stack"><active_stack info="Contains functions not left with a return"><test><Tester/></test></active_stack></active_stack>"""
        # The node text should have a Tester node added
        xmllogging.exit_active_stack(an_object)

        self.assertEqual(stack_text, goal_text,
            "THe created xml structure is not correct")
