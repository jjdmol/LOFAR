from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile

#imports from fixture:
import lofarpipe.support.pipelinexml as pipexml
import xml.dom.minidom as xml


class pipelinexmlTest(unittest.TestCase):
    def __init__(self, arg):
        super(pipelinexmlTest, self).__init__(arg)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    # reading and writing of parset is extremely limited: The parameterset
    # is part of the mucked fixture.
    # The lofar.parameterset has a the todict function. 
    # This means that all test start and leave from this interface.

    def test_read_parset_to_dict(self):
        # write a tempparset with test data
        testdata = \
"""
#Comment 
key1='value' #end of line comment 
key2= value \  #end of line comment 
additional data 
key3=value #\end of line comment 
key4=value=value
   #randomdata this is not parsed
    key5 = value                
"""
        expected_dict = {}
        expected_dict["key1"] = "'value'"
        expected_dict["key2"] = "value additional data"
        expected_dict["key3"] = "value"
        expected_dict["key4"] = "value=value"
        expected_dict["key5"] = "value"

        temp_dir = tempfile.mkdtemp()
        temp_parset_path = os.path.join(temp_dir, "parset.par")
        fp = open(temp_parset_path, 'w')
        fp.write(testdata)
        fp.close

        fp = open(temp_parset_path, 'r')

        dicted_parset = pipexml._read_parset_to_dict(temp_parset_path)
        for key in expected_dict.iterkeys():
            message = "dict did not contain expected key {0} \n {1}".format(
                    key, dicted_parset)
            self.assertTrue(dicted_parset.has_key(key), message)

            message = "retrieved value based on key {0} is not the correct: \n"\
               " >{1}< != >{2}< ".format(key, dicted_parset[key], expected_dict[key])

            self.assertTrue(dicted_parset[key] == expected_dict[key], message)

    def test_convert_dict_to_xml_node_multi_toplevel(self):
        input_dict = {}
        input_dict["node1.node21.leaf1"] = "leaf1"
        input_dict["node1.node21.leaf2"] = "leaf2"
        input_dict["node1.node22.leaf3"] = "leaf3"
        input_dict["node1.node3.leaf4"] = "leaf4"
        input_dict["node1.node21"] = "leaf_node"
        input_dict["parset_leaf"] = "leaf5"
        toplevel_name = "parset"

        expected_output = """<parset parset_leaf="leaf5"><node1 node21="leaf_node"><node3 leaf4="leaf4"/><node22 leaf3="leaf3"/><node21 leaf1="leaf1" leaf2="leaf2"/></node1></parset>"""
        xml = pipexml._convert_dict_to_xml_node(input_dict, toplevel_name)

        message = "_convert_key_to_xml_tree returned an unexpected value: expected, received\n"\
            "{0} \n{1}".format(expected_output, xml.toxml())
        self.assertTrue(expected_output == xml.toxml(), message)

    def test_convert_dict_to_xml_node_single_toplevel(self):
        input_dict = {}
        input_dict["node1.node21.leaf1"] = "leaf1"
        input_dict["node1.node21.leaf2"] = "leaf2"
        input_dict["node1.node22.leaf3"] = "leaf3"
        input_dict["node1.node3.leaf4"] = "leaf4"
        input_dict["node1.node21"] = "leaf_node"

        expected_output = """<node1 node21="leaf_node"><node21 leaf1="leaf1" leaf2="leaf2"/><node22 leaf3="leaf3"/><node3 leaf4="leaf4"/></node1>"""
        xml = pipexml._convert_dict_to_xml_node(input_dict)

        message = "_convert_key_to_xml_tree returned an unexpected value: expected, received\n"\
            "{0} \n{1}".format(expected_output, xml.toxml())
        self.assertTrue(expected_output == xml.toxml(), message)

    def test_convert_xml_node_to_dict(self):
        # TODO: brittle test:  it uses _convert_dict_to_xml_node
        input_dict = {}
        input_dict["node1.node21.leaf1"] = "leaf1"
        input_dict["node1.node21.leaf2"] = "leaf2"
        input_dict["node1.node22.leaf3"] = "leaf3"
        input_dict["node1.node3.leaf4"] = "leaf4"
        input_dict["node1.node21"] = "leaf_node"
        input_dict["parset_leaf"] = "leaf5"

        xml = pipexml._convert_dict_to_xml_node(input_dict)

        output_dict = {}
        pipexml._convert_xml_node_to_dict(xml.firstChild, [], output_dict)

        for key in input_dict.keys():
            if key == "parset_leaf":
                self.assertFalse(output_dict.has_key(key), "The output dict"
                        "contained a parset leaf: this should not be returned"
                        "in this test case")
                continue

            self.assertTrue(output_dict.has_key(key),
                "The generated dict did not contain the expected key:\n {0}\n{1}".format(
                        input_dict, output_dict))
            self.assertTrue(output_dict[key] == input_dict[key],
                "the received value for key {0} was not equal: {1}, {2}".format(
                    key, input_dict[key], output_dict[key]))

    def test_convert_xml_to_dict_node(self):
        input_dict = {}
        input_dict["node1.node21.leaf1"] = "leaf1"
        input_dict["node1.node21.leaf2"] = "leaf2"
        input_dict["node1.node22.leaf3"] = "leaf3"
        input_dict["node1.node3.leaf4"] = "leaf4"
        input_dict["node1.node21"] = "leaf_node"

        toplevel_name = "a_name"

        xml_node = pipexml._convert_dict_to_xml_node(input_dict,)


        output_dict = {}
        output_dict = pipexml._convert_xml_to_dict(xml_node)

        for key in input_dict.keys():
            if key == "parset_leaf":
                self.assertFalse(output_dict.has_key(key), "The output dict"
                        "contained a parset leaf: this should not be returned"
                        "in this test case")
                continue
            self.assertTrue(output_dict.has_key(key),
                "The generated dict did not contain the expected key:\n {0}\n{1}".format(
                        input_dict, output_dict))
            self.assertTrue(output_dict[key] == input_dict[key],
                "the received value for key {0} was not equal: {1}, {2}".format(
                    key, input_dict[key], output_dict[key]))

    def test_convert_xml_to_dict_document(self):
        input_dict = {}
        input_dict["node1.node21.leaf1"] = "leaf1"
        input_dict["node1.node21.leaf2"] = "leaf2"
        input_dict["node1.node22.leaf3"] = "leaf3"
        input_dict["node1.node3.leaf4"] = "leaf4"
        input_dict["node1.node21"] = "leaf_node"

        toplevel_name = "a_name"
        xml_node = pipexml._convert_dict_to_xml_node(input_dict)


        xml_document = xml.Document()
        xml_root = xml_document.createElement("root")
        xml_root.appendChild(xml_node)
        xml_document.appendChild(xml_root)
        output_dict = {}
        output_dict = pipexml._convert_xml_to_dict(xml_document)

        for key in input_dict.keys():
            if key == "parset_leaf":
                self.assertFalse(output_dict.has_key(key), "The output dict"
                        "contained a parset leaf: this should not be returned"
                        "in this test case")
                continue
            self.assertTrue(output_dict.has_key(key),
                "The generated dict did not contain the expected key:\n {0}\n{1}".format(
                        input_dict, output_dict))
            self.assertTrue(output_dict[key] == input_dict[key],
                "the received value for key {0} was not equal: {1}, {2}".format(
                    key, input_dict[key], output_dict[key]))


    def test_dict_to_xml_elementsbyname_bug(self):
        """
        In the dict to xml conversion the matching of leaf/node names with the parent is done using
        getElementsByTagName which also produces matches with grandchildren.
        Adding an test of match is indeed parent lets this test succeed
        """
        # write a tempparset with test data
        input_dict = {}
        input_dict["ObsSW.Observation.ObservationControl.PythonControl.BBS.Step.DefaultBBSStep[0].Solve.CellSize.Freq"] = "4"
        input_dict["ObsSW.Observation.ObservationControl.PythonControl.BBS.Step.DefaultBBSStep[0].Solve.Resample.CellSize.Freq"] = "0"
        input_dict["ObsSW.Observation.ObservationControl.PythonControl.BBS.Step.DefaultBBSStep[0].Solve.Resample.CellSize.Time"] = "1"
        input_dict["ObsSW.Observation.ObservationControl.PythonControl.BBS.Step.DefaultBBSStep[0].Solve.CellSize.Time"] = "5"


        xml_node = pipexml._convert_dict_to_xml_node(input_dict)
        output_dict = pipexml._convert_xml_to_dict(xml_node)

        for key in input_dict.iterkeys():
            message = "dict did not contain expected key {0} \n {1}".format(
                    key, output_dict)
            self.assertTrue(output_dict.has_key(key), message)

            message = "retrieved value based on key {0} is not the correct: \n"\
               " >{1}< != >{2}< \n output: \n {3} \n expected: \n {4} ".format(key, output_dict[key], input_dict[key], output_dict, input_dict)

            self.assertTrue(output_dict[key] == input_dict[key], message)

    def test_convert_value_with_quotes(self):
        # write a tempparset with test data
        input_dict = {}
        input_dict["test.key1"] = """"'value"'"""

        xml_node = pipexml._convert_dict_to_xml_node(input_dict)

        #self.assertTrue(False, input_dict)
        output_dict = pipexml._convert_xml_to_dict(xml_node)

        for key in input_dict.iterkeys():
            message = "dict did not contain expected key {0} \n {1}".format(
                    key, output_dict)
            self.assertTrue(output_dict.has_key(key), message)

            message = "retrieved value based on key {0} is not the correct: \n"\
               " >{1}< != >{2}< ".format(key, output_dict[key], input_dict[key])

            self.assertTrue(output_dict[key] == input_dict[key], message)


if __name__ == "__main__":
    import xmlrunner
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='result.xml'))
