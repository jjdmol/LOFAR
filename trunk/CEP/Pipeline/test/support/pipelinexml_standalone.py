import sys

import lofarpipe.support.pipelinexml as pipexml


if __name__ == "__main__":
    fp = None
    try:
        arg1 = sys.argv[1:2]

    except:
        print """ Usage: python pipelinexml_standalone.py [x.parset]
        Converts x.parset and writes x.parset.xml
        Then it converts to xml back to parset and writes it to x.parset.out                
        """
    # Open parset with default document name parset
    xml_node = pipexml.open_parset_as_xml_node(arg1[0])

    # open file write xml node to pretty xml
    fp = open(arg1[0] + ".xml", 'w')
    fp.write(xml_node.toprettyxml())
    fp.close()

    # convert back to parset and save
    pipexml.write_xml_as_parset(xml_node, arg1[0] + ".out", "dict")
