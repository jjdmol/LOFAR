import sys, cStringIO

def htmlify(S, nbspFlag = True):
    S = S.replace("&", "&amp;")
    S = S.replace("<", "&lt;")
    S = S.replace(">", "&gt;")
    if nbspFlag:
        S = S.replace(" ", "&nbsp;")
    S = S.replace("\n", "<br />")
    return S

class LofarComponent:
    def name(self):
        return self._name
    def html(self, first_line_indent = '', line_indent = '', indent = '    '):
        return ""
        save_stdout = sys.stdout
        sys.stdout = cStringIO.StringIO()
        self.print_html(first_line_indent, line_indent, indent)
        result = sys.stdout.getvalue()
        sys.stdout.close()
        sys.stdout = save_stdout
        return result
    def print_html(self, first_line_indent = '', line_indent = '', indent = '    '):
        print first_line_indent + '<table class="%s">' % self._classify
        print line_indent + indent + '<tr>'
        if len(self._children)>1:
            colspan_opt = ' colspan="%d"' % len(self._children)
        else:
            colspan_opt = ''
        print line_indent + 2 * indent + '<th%s>' % colspan_opt + '<a href="">' + htmlify(self.name()) + '</a>' + '</th>'
        print line_indent + indent + '</tr>'
        if len(self._children)>0:
            if self._children_htmlstyle == "horizontal":
                print line_indent + indent + '<tr>'
                for child in self._children:
                    print line_indent + 2 * indent + '<td>'
                    child.print_html(line_indent + 3 * indent, line_indent + 3 * indent, indent)
                    print line_indent + 2 * indent + '</td>'
                print line_indent + indent + '</tr>'
            elif self._children_htmlstyle == "vertical":
                for child in self._children:
                    print line_indent + indent + '<tr>'
                    print line_indent + 2 * indent + '<td>'
                    child.print_html(line_indent + 3 * indent, line_indent + 3 * indent, indent)
                    print line_indent + 2 * indent + '</td>'
                    print line_indent + indent + '</tr>'
            else:
                assert False
        print line_indent + '</table>'

class FTS(LofarComponent):
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "station"
        self._children_htmlstyle = "vertical"
        self._children = [
            RSPBoard(None, "RSP0"),
            RSPBoard(None, "RSP1"),
            PowerSupplyUnit(None,"POWER"),
            GPSUnit(None,"GPS")
        ]

class RemoteStation(LofarComponent):
    """LOFAR Remote Station"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "station"
        self._children_htmlstyle = "horizontal"
        self._children = [
            Rack(None, "Rack #1", [SubRack(None, "SubRack0"), SubRack(None, "SubRack1"), SubRack(None, "SubRack2")]),
            Rack(None, "Rack #2", [GPSUnit(None, "GPS Unit"), PowerSupplyUnit(None, "Power Supply")]),
            Rack(None, "Rack #3", [SubRack(None, "SubRack3"), SubRack(None, "SubRack4"), SubRack(None, "SubRack5")])
        ]

class Rack(LofarComponent):
    """19\" Rack"""
    def __init__(self, version, name, children):
        self._version = version
        self._name = name
        self._classify = "rack"
        self._children_htmlstyle = "vertical"
        self._children = children

class SubRack(LofarComponent):
    """Sub-Rack (a.k.a. Crate)"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "subrack"
        self._children_htmlstyle = "vertical"
        self._children = [
            RSPBoard(None, "RSP0"),
            RSPBoard(None, "RSP1"),
            RSPBoard(None, "RSP2"),
            RSPBoard(None, "RSP3")
        ]

class GPSUnit(LofarComponent):
    """GPS Unit"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "subrack"
        self._children = []

class PowerSupplyUnit(LofarComponent):
    """Power Supply"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "subrack"
        self._children_htmlstyle = "vertical"
        self._children = []

class RSPBoard(LofarComponent):
    """Remote Station Processing (RSP) Board"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "rsp"
        self._children_htmlstyle = "horizontal"
        self._children = [
            AntennaProcessor(None, "AP0"),
            AntennaProcessor(None, "AP1"),
            AntennaProcessor(None, "AP2"),
            AntennaProcessor(None, "AP3")
        ]

class AntennaProcessor(LofarComponent):
    """Antenna Processor (AP)"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "ap"
        self._children_htmlstyle = "horizontal"
        self._children = [
            ReceiverUnit(None, "RCU x"),
            ReceiverUnit(None, "RCU y")
        ]

class ReceiverUnit(LofarComponent):
    """Receiver Unit (RCU)"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "rcu"
        self._children_htmlstyle = "horizontal"
        self._children = [LowBandAntenna(None, "L"), HighBandAntenna(None, "H")]

class LowBandAntenna(LofarComponent):
    """Low-Band Antenna (LBA)"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "antenna"
        self._children = []

class HighBandAntenna(LofarComponent):
    """High-Band Antenna (HBA)"""
    def __init__(self, version, name):
        self._version = version
        self._name = name
        self._classify = "antenna"
        self._children = []

if __name__ == "__main__":
    rs = RemoteStation(version = None, name = "FTS-2")
    rs.print_html()
