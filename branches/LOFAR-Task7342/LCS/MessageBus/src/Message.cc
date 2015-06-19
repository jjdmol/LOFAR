//#  MessageContent.cc: one_line_description
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/StringUtil.h>
#include <MessageBus/Message.h>

#ifdef HAVE_QPID
#include <qpid/types/Uuid.h>
#endif

#ifdef HAVE_LIBXMLXX
#include <sstream>
#include <libxml++/parsers/domparser.h>
#include <libxml++/nodes/textnode.h>

using namespace xmlpp;
#endif

#include <time.h>


namespace LOFAR {
  using namespace StringUtil;

/*
 * The template for the LOFAR message format.
 */
const string LOFAR_MSG_TEMPLATE = "\
<message>\n\
   <header>\n\
      <system></system>\n\
      <version></version>\n\
      <protocol>\n\
         <name></name>\n\
         <version></version>\n\
      </protocol>\n\
      <source>\n\
         <name></name>\n\
         <user></user>\n\
         <uuid></uuid>\n\
         <timestamp></timestamp>\n\
         <summary></summary>\n\
      </source>\n\
      <ids>\n\
         <momid></momid>\n\
         <sasid></sasid>\n\
      </ids>\n\
   </header>\n\
   <payload></payload>\n\
</message>";

static string _timestamp() {
  // Get now (in seconds since epoch)
  time_t now = time(NULL);

  // Dissect into components (year, month, etc)
  struct tm now_tm;
  gmtime_r(&now, &now_tm);

  // Convert to string
  char buffer[64];
  if (strftime(buffer, sizeof buffer, "%FT%T", &now_tm) == 0)
    buffer[0] = 0;

  return buffer;
}

static string _uuid() {
  qpid::types::Uuid uuid(true);
  return uuid.str();
}

MessageContent::MessageContent()
{
  initContent(LOFAR_MSG_TEMPLATE);
  addProperties();
}

MessageContent::MessageContent(const std::string &from,
				 const std::string &forUser,
				 const std::string &summary,
				 const std::string &protocol,
				 const std::string &protocolVersion,
				 const std::string &momid,
				 const std::string &sasid)
{
  initContent(LOFAR_MSG_TEMPLATE);
  addProperties();

  this->system          = LOFAR::system;
  this->headerVersion   = LOFAR::headerVersion;

  this->protocol        = protocol;
  this->protocolVersion = protocolVersion;
  this->name            = from;
  this->user            = forUser;
  this->uuid            = _uuid();
  this->summary         = summary;
  this->timestamp       = _timestamp();
  this->momid           = momid;
  this->sasid           = sasid;
}

MessageContent::MessageContent(const qpid::messaging::Message &qpidMsg)
{
  initContent(qpidMsg.getContent());
  addProperties();
}

MessageContent::~MessageContent()
{
}

MessageContent::MessageContent(const MessageContent &other)
{
  initContent(other.getContent());
  addProperties();
}

void MessageContent::initContent(const std::string &content)
{
#ifdef HAVE_LIBXMLXX
  itsParser.parse_memory(content);
  itsDocument = itsParser.get_document();
#else
  itsContent = content;
#endif
}

std::string MessageContent::getContent() const
{
#ifdef HAVE_LIBXMLXX
  return itsDocument->write_to_string_formatted();
#else
  return itsContent;
#endif
}

void MessageContent::addProperties()
{
  system         .attach(this, "message/header/system");
  headerVersion  .attach(this, "message/header/version");

  protocol       .attach(this, "message/header/protocol/name");
  protocolVersion.attach(this, "message/header/protocol/version");

  name           .attach(this, "message/header/source/name");
  user           .attach(this, "message/header/source/user");
  uuid           .attach(this, "message/header/source/uuid");

  summary        .attach(this, "message/header/source/summary");
  timestamp      .attach(this, "message/header/source/timestamp");

  momid          .attach(this, "message/header/ids/momid");
  sasid          .attach(this, "message/header/ids/sasid");

  payload        .attach(this, "message/payload");
  header         .attach(this, "message/header");
}

qpid::messaging::Message MessageContent::qpidMsg() const {
  qpid::messaging::Message qpidMsg;

  qpidMsg.setContent(getContent());
  qpidMsg.setContentType("text/plain"); // Don't use text/xml, to prevent QPID from nosing in our message and possibly complaining
  qpidMsg.setDurable(true);

  return qpidMsg;
}

void MessageContent::setXMLPayload (const std::string         &payload)
{
  insertXML("message/payload", payload);
}

void MessageContent::setTXTPayload (const std::string         &payload)
{
  setXMLvalue("message/payload", payload);
}

std::string MessageContent::short_desc() const
{
  return formatString("[%s] [sasid %s] %s", uuid.get().c_str(), sasid.get().c_str(), summary.get().c_str());
}

std::ostream& MessageContent::print (std::ostream& os) const
{
    os << "system         : " << system << " " << headerVersion << endl;
    os << "protocol       : " << protocol << " " << protocolVersion << endl;
    os << "summary        : " << summary << endl;
    os << "timestamp      : " << timestamp << endl;
    os << "source (name)  : " << name << endl;
    os << "user           : " << user << endl;
    os << "uuid           : " << uuid << endl;
    os << "momid          : " << momid << endl;
    os << "sasid          : " << sasid << endl;
    os << "payload        : " << payload << endl;
    return (os);
}

string MessageContent::getXMLvalue(const string& key) const
{
#ifdef HAVE_LIBXMLXX
  Element *e = getXMLnode(key);

  // Extract the text, if any
  TextNode *t = e->get_child_text();
  if (!t) return "";

  return t->get_content();
#else
  // get copy of content
  vector<string>  labels = split(key, '/');

  // loop over subkeys
  string::size_type  offset = 0;
  string::size_type  begin = string::npos;
  string::size_type  end = string::npos;
  string        startTag;
  for (size_t i = 0; i <  labels.size(); ++i) {
    // define tags to find
    startTag = string("<"+labels[i]+">");
    // search begin tag
    begin  = itsContent.find(startTag, offset);
    if (begin == string::npos) {
      return ("???");
    }
    offset = begin;
  }
  // search end tag
  string stopTag ("</"+labels[labels.size()-1]+">");
  begin+=startTag.size();
  end = itsContent.find(stopTag, begin);
  if (end == string::npos) {
    return ("???");
  }
  return (itsContent.substr(begin, end - begin));
#endif
}

void MessageContent::setXMLvalue(const string& key, const string &data)
{
#ifdef HAVE_LIBXMLXX
  Element *e = getXMLnode(key);

  e->set_child_text(data);
#else
  // get copy of content
  vector<string>  labels = split(key, '/');

  // loop over subkeys
  string::size_type  offset = 0;
  string::size_type  begin = string::npos;
  string::size_type  end = string::npos;
  string        startTag;
  for (size_t i = 0; i <  labels.size(); ++i) {
    // define tags to find
    startTag = string("<"+labels[i]+">");
    // search begin tag
    begin  = itsContent.find(startTag, offset);
    if (begin == string::npos) {
      THROW(MessageContentException, "XML element not found (could not find begin tag): " << key);
    }
    offset = begin;
  }
  // search end tag
  string stopTag ("</"+labels[labels.size()-1]+">");
  begin+=startTag.size();
  end = itsContent.find(stopTag, begin);
  if (end == string::npos) {
    THROW(MessageContentException, "XML element not found (could not find end tag): " << key);
  }

  itsContent.replace(begin, end - begin, data);
#endif
}

void MessageContent::insertXML(const string &key, const string &xml)
{
#ifdef HAVE_LIBXMLXX
  // Find insert spot
  Element *e = getXMLnode(key);
  if (!e) return;

  // Parse provided XML
  DomParser parser;
  parser.parse_memory(xml);

  Document *document = parser.get_document();
  if (!document) return;

  Element *root = document->get_root_node();
  if (!root) return;

  // Insert the XML into our document
  e->import_node(root);
#else
  setXMLvalue(key, xml);
#endif
}

#ifdef HAVE_LIBXMLXX
Element *MessageContent::getXMLnode(const string &name) const
{
  Element *root = itsDocument->get_root_node();

  if (!root) {
    // Document is broken
    THROW(MessageContentException, "Document is broken");
  }

  // assume key is an XPath relative to root, see http://www.w3schools.com/xpath/xpath_syntax.asp
  NodeSet nodeset = root->find("/"+name);
  if (nodeset.empty()) {
    // Element not found
    THROW(MessageContentException, "XML element not found: /" << name);
  }

  Element *e = dynamic_cast<Element*>(nodeset[0]);

  if (!e) {
    // Key points to a special element
    THROW(MessageContentException, "XML element not a text element: /" << name);
  }

  return e;
}
#endif

Message::Message(const MessageContent &content)
:
  itsQpidMsg(content.qpidMsg())
{
}


} // namespace LOFAR
