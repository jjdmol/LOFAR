//# pkgdep.cc: Find dependencies of packages
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>

using namespace std;

// Define the struct to hold the usage.
struct Used
{
  // Constructor.
  Used() : itsNused(0) {};
  // Number of times the package is used.
  int          itsNused;
  // Names of the packages being used.
  set<string> itsUses;
};

// Define the map to hold the usage.
typedef map<string,Used> UsedMap;

// Remove the basename part.
string baseName(const string& name, bool strip)
{
  if (!strip) {
    return name;
  }
  string::size_type pos = name.rfind('/');
  if (pos == string::npos) {
    return name;
  }
  return name.substr(pos+1);
}

// Replace slashes by underscores.
string replaceSlash (const string& str)
{
  string out = str;
  string::size_type idx = out.find('/');
  while (idx != string::npos) {
    out.replace (idx, 1, 1, '_');
    idx = out.find('/');
  }
  return out;
}

// Write the dependency tree in ASCII format.
void writeASCII (ostream& os, const string& pkg, UsedMap& dep,
		 const string& indent,
		 int depth, int maxdepth, bool strip)
{
  os << indent << baseName(pkg,strip) << endl;
  // write used packages if any and if maxdepth not reached.
  if (maxdepth < 0  ||  depth < maxdepth) {
    string newIndent = indent + ' ';
    set<string> uses = dep[pkg].itsUses;
    for (set<string>::const_iterator iter = uses.begin();
	 iter != uses.end();
	 ++iter) {
      writeASCII (os, *iter, dep, newIndent, depth+1, maxdepth, strip);
    }
  }
}

// Write JavaScript header.
void writeJSHeader (const string& type)
{
  cout << "<html>" << endl;
  cout << "<head>" << endl;
  cout << "<script src='lib/TreeMenu.js' language='JavaScript' type='text/javascript'></script>" << endl;
  cout << "<title>LOFAR Dependency Tree (" << type << ")</title>" << endl;
  cout << "</head>" << endl;
  cout << "<body>" << endl;
  cout << "<center>" << endl;
  cout << "<h1>LOFAR Dependency Tree (" << type << ")</h1>" << endl;
  cout << "</center>" << endl;
  cout << "<script language='javascript' type='text/javascript'>" << endl;
  cout << "  objTreeMenu_1 = new TreeMenu('images', 'objTreeMenu_1', '_self', 'treeMenuDefault', true, false);" << endl;
  cout << "  newNode = objTreeMenu_1.addItem(new TreeNode('Lofar dependencies', 'folder.gif', null, false, true, ''));" << endl;
}

// Write JavaScript footer.
void writeJSFooter()
{
  cout << "  objTreeMenu_1.drawMenu();" << endl;
  cout << "  objTreeMenu_1.resetBranches();" << endl;
  cout << "</script></body>" << endl;
  cout << "</html>" << endl;
}

// Write the dependency tree in JavaScript format.
void writeJS (const string& pkg, UsedMap& dep, const string& parent,
	      int depth, int maxdepth, int seqnr)
{
  // Form the name for this node.
  ostringstream oss;
  oss << parent << '_' << seqnr+1;
  // Get children.
  set<string> uses = dep[pkg].itsUses;
  cout << "newNode" << oss.str()
       << "= newNode" << parent << ".addItem(new TreeNode('"
       << pkg << "', '";
  if (uses.empty()) {
    cout << "item";
  } else {
    cout << "folder";
  }
  cout << ".gif', null, false, true, ''));" << endl;
  // Write used packages if any and if maxdepth not reached.
  if (maxdepth < 0  ||  depth < maxdepth) {
    int newSeqnr = 0;
    for (set<string>::const_iterator iter = uses.begin();
	 iter != uses.end();
	 ++iter, ++newSeqnr) {
      writeJS (*iter, dep, oss.str(), depth+1, maxdepth, newSeqnr);
    }
  }
}

void writeXHTMLHeader()
{
  cout << "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>" << endl;
  cout << "  <head>" << endl;
  cout << "    <meta http-equiv='Content-Type' content='text/xhtml;charset='iso-8859-1' />" << endl;
  cout << "    <meta http-equiv='Content-Style-Type' content='text/css' />" << endl;
  cout << "    <meta http-equiv='Content-Language' content='en' />" << endl;
  cout << "    <link rel='stylesheet' href='doxygen.css'>" << endl;
  cout << "    <title>TreeView</title>" << endl;
  cout << "    <style type='text/css'>" << endl;
  cout << "    <!--" << endl;
  cout << "    .directory { font-size: 10pt; font-weight: bold; }" << endl;
  cout << "    .directory h3 { margin: 0px; margin-top: 1em; font-size: 11pt; }" << endl;
  cout << "    .directory p { margin: 0px; white-space: nowrap; }" << endl;
  cout << "    .directory div { display: none; margin: 0px; }" << endl;
  cout << "    .directory img { vertical-align: middle; }" << endl;
  cout << "    -->" << endl;
  cout << "    </style>" << endl;
  cout << "    <script type='text/javascript'>" << endl;
  cout << "    <!-- // Hide script from old browsers" << endl;
  cout << "    " << endl;
  cout << "    function findChildNode(node, name) " << endl;
  cout << "    {" << endl;
  cout << "      var temp;" << endl;
  cout << "      if (node == null) " << endl;
  cout << "      {" << endl;
  cout << "        return null;" << endl;
  cout << "      } " << endl;
  cout << "      node = node.firstChild;" << endl;
  cout << "      while (node != null) " << endl;
  cout << "      {" << endl;
  cout << "        if (node.nodeName == name) " << endl;
  cout << "        {" << endl;
  cout << "          return node;" << endl;
  cout << "        }" << endl;
  cout << "        temp = findChildNode(node, name);" << endl;
  cout << "        if (temp != null) " << endl;
  cout << "        {" << endl;
  cout << "          return temp;" << endl;
  cout << "        }" << endl;
  cout << "        node = node.nextSibling;" << endl;
  cout << "      }" << endl;
  cout << "      return null;" << endl;
  cout << "    }" << endl;
  cout << "" << endl;
  cout << "    function toggleFolder(id, imageNode) " << endl;
  cout << "    {" << endl;
  cout << "      var folder = document.getElementById(id);" << endl;
  cout << "      var l = 0;" << endl;
  cout << "      var vl = 'ftv2vertline.png';" << endl;
  cout << "      if (imageNode != null && imageNode.nodeName != 'IMG') " << endl;
  cout << "      {" << endl;
  cout << "        imageNode = findChildNode(imageNode, 'IMG');" << endl;
  cout << "        if (imageNode!=null) l = imageNode.src.length;" << endl;
  cout << "      }" << endl;
  cout << "      if (folder == null) " << endl;
  cout << "      {" << endl;
  cout << "      } " << endl;
  cout << "      else if (folder.style.display == 'block') " << endl;
  cout << "      {" << endl;
  cout << "        while (imageNode != null && " << endl;
  cout << "               imageNode.src.substring(l-vl.length,l) == vl)" << endl;
  cout << "        {" << endl;
  cout << "          imageNode = imageNode.nextSibling;" << endl;
  cout << "          l = imageNode.src.length;" << endl;
  cout << "        }" << endl;
  cout << "        if (imageNode != null) " << endl;
  cout << "        {" << endl;
  cout << "          l = imageNode.src.length;" << endl;
  cout << "          imageNode.nextSibling.src = 'ftv2folderclosed.png';" << endl;
  cout << "          if (imageNode.src.substring(l-13,l) == 'ftv2mnode.png')" << endl;
  cout << "          {" << endl;
  cout << "            imageNode.src = 'ftv2pnode.png';" << endl;
  cout << "          }" << endl;
  cout << "          else if (imageNode.src.substring(l-17,l) == 'ftv2mlastnode.png')" << endl;
  cout << "          {" << endl;
  cout << "            imageNode.src = 'ftv2plastnode.png';" << endl;
  cout << "          }" << endl;
  cout << "        }" << endl;
  cout << "        folder.style.display = 'none';" << endl;
  cout << "      } " << endl;
  cout << "      else " << endl;
  cout << "      {" << endl;
  cout << "        while (imageNode != null && " << endl;
  cout << "               imageNode.src.substring(l-vl.length,l) == vl)" << endl;
  cout << "        {" << endl;
  cout << "          imageNode = imageNode.nextSibling;" << endl;
  cout << "          l = imageNode.src.length;" << endl;
  cout << "        }" << endl;
  cout << "        if (imageNode != null)" << endl; 
  cout << "        {" << endl;
  cout << "          l = imageNode.src.length;" << endl;
  cout << "          imageNode.nextSibling.src = 'ftv2folderopen.png';" << endl;
  cout << "          if (imageNode.src.substring(l-13,l) == 'ftv2pnode.png')" << endl;
  cout << "          {" << endl;
  cout << "            imageNode.src = 'ftv2mnode.png';" << endl;
  cout << "          }" << endl;
  cout << "          else if (imageNode.src.substring(l-17,l) == 'ftv2plastnode.png')" << endl;
  cout << "          {" << endl;
  cout << "            imageNode.src = 'ftv2mlastnode.png';" << endl;
  cout << "          }" << endl;
  cout << "        }" << endl;
  cout << "        folder.style.display = 'block';" << endl;
  cout << "      }" << endl;
  cout << "    }" << endl;
  cout << "" << endl;
  cout << "    // End script hiding --> " << endl;       
  cout << "    </script>" << endl;
  cout << "  </head>" << endl;
  cout << "" << endl;
  cout << "  <body>" << endl;
  cout << "    <div class='directory'>" << endl;
  cout << "      <h3>LCS/Common</h3>" << endl;
  cout << "      <div style='display: block;'>" << endl;
}

void writeXHTMLFooter()
{
  cout << "      </div>" << endl;
  cout << "    </div>" << endl;
  cout << "  </body>" << endl;
  cout << "</html>" << endl;
}

// Write the dependency tree in XHTML format.
void writeXHTML (const string& pkg, UsedMap& dep, const string& parent,
		 int depth, int maxdepth, int seqnr)
{
  // Form the name for this node.
  ostringstream oss;
  oss << parent << '_' << seqnr+1;
  // Get children.
  set<string> uses = dep[pkg].itsUses;
  cout << "<p>";
  for (int i=0; i<depth; ++i) {
    cout << "<img src='ftv2vertline.png' alt='|' width=16 height=22 />";
  }
  if (uses.empty()) {
    cout << "<img src='ftv2node.png' alt='o' width=16 height=22 />";
    cout << "<img src='ftv2doc.png' alt='*' width=24 height=22 />";
    cout << pkg << "</p>" << endl;
  } else {
    cout << "<img src='ftv2pnode.png' alt='o' width=16 height=22 onclick='toggleFolder(";
    cout << '"' << "node" << oss.str() << '"';
    cout << ", this)'/>";
    cout << "<img src='ftv2folderclosed.png' alt='+' width=24 height=22 onclick='toggleFolder(";
    cout << '"' << "node" << oss.str() << '"';
    cout << ", this)'/>" << pkg << "</p>" << endl;
    cout << "<div id='node" << oss.str() << "'>" << endl;
    // Write used packages if any and if maxdepth not reached.
    if (maxdepth < 0  ||  depth < maxdepth) {
      int newSeqnr = 0;
      for (set<string>::const_iterator iter = uses.begin();
	   iter != uses.end();
	   ++iter, ++newSeqnr) {
	writeXHTML (*iter, dep, oss.str(), depth+1, maxdepth, newSeqnr);
      }
    }
    cout << "</div>" << endl;
  }
}

// Determine all dependencies.
void findFlatDep (const string& pkg, UsedMap& dep, set<string>& flatUses)
{
  set<string> uses = dep[pkg].itsUses;
  for (set<string>::const_iterator iter = uses.begin();
       iter != uses.end();
       ++iter) {
    flatUses.insert (*iter);
    findFlatDep (*iter, dep, flatUses);
  }
}

int main(int argc, const char* argv[])
{
  if (argc < 2) {
    cerr << "Use as:   pkgdep inputfile [flat|maxdepth=-1] [ascii|xhtml|js]"
	 << "[strip] [top] [split=ext]"
	 << endl;
    return 1;
  }
  enum OutType {ASCII,XHTML,JS};
  bool flat=false;
  OutType outtype = JS;
  bool top=false;
  bool strip=false;
  bool split=false;
  string ext="";
  int maxdepth=-1;
  for (int i=2; i<argc; ++i) {
    string arg = argv[i];
    string val;
    string::size_type idx = arg.find('=');
    if (idx != string::npos) {
      val = arg.substr(idx+1);
      arg = arg.substr(0, idx);
    }
    if (arg == "ascii") {
      outtype = ASCII;
    } else if (arg == "js") {
      outtype = JS;
    } else if (arg == "xhtml") {
      outtype = XHTML;
    } else if (arg == "top") {
      top = true;
    } else if (arg == "strip") {
      strip = true;
    } else if (arg == "split") {
      split = true;
      ext = val;
    } else if (arg == "flat") {
      flat = true;
      maxdepth = 1;
    } else {
      istringstream istr(argv[2]);
      istr >> maxdepth;
    }
  }

  // Hold the dependencies and the used in.
  UsedMap dep;

  // Read all packages and record which packages it uses and how often used.
  ifstream ifs(argv[1]);
  string pkg1, pkg2;
  while (ifs) {
    ifs >> pkg1;
    if (ifs) {
      ifs >> pkg2;
      dep[pkg1].itsUses.insert (pkg2);
      dep[pkg2].itsNused++;
    }
  }

  UsedMap* depPtr = &dep;
  // If flat dependencies have to be determined, do so for all packages.
  UsedMap flatdep;
  if (flat) {
    for (UsedMap::const_iterator iter = dep.begin();
	 iter != dep.end();
	 ++iter) {
      findFlatDep (iter->first, dep, flatdep[iter->first].itsUses);
    }
    depPtr = &flatdep;
  }

  if (outtype == JS) {
    string name(argv[1]);
    writeJSHeader(name.substr(name.find('.') + 1));
  } else if (outtype == XHTML) {
    writeXHTMLHeader();
  }
  // Write the dependencies starting at all root packages
  // (i.e. packages not used by others).
  int seqnr = 0;
  for (UsedMap::const_iterator iter = depPtr->begin();
       iter != depPtr->end();
       ++iter, ++seqnr) {
    if (!top  ||  iter->second.itsNused == 0) {
      ostream* ptr = &cout;
      if (split) {
	string name = replaceSlash(iter->first) + ext;
	cerr << name << endl;
	ptr = new ofstream(name.c_str());
      }
      switch (outtype) {
      case ASCII:
	writeASCII (*ptr, iter->first, *depPtr, "", 0, maxdepth, strip);
	break;
      case JS:
	writeJS (iter->first, *depPtr, "", 0, maxdepth, seqnr);
	break;
      case XHTML:
	writeXHTML (iter->first, *depPtr, "", 0, maxdepth, seqnr);
	break;
      }
      if (ptr != &cout) {
	delete ptr;
      }
    }
  }
  if (outtype == JS) {
    writeJSFooter();
  } else if (outtype == XHTML) {
    writeXHTMLFooter();
  }
}
