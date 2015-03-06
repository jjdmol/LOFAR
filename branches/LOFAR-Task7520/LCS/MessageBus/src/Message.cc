//#  Message.cc: one_line_description
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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
#include <MessageBus/Exceptions.h>
#include <MessageBus/Message.h>

#include <qpid/types/Uuid.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <time.h>

// TODO; Debugging
#include <iostream>



namespace LOFAR {
  using namespace StringUtil;

const string LOFAR_MSG_TEMPLATE = "\
<message>\n\
   <header>\n\
      <system>LOFAR</system>\n\
      <version>1.0.0</version>\n\
      <protocol>\n\
         <name>%s</name>\n\
         <version>%s</version>\n\
      </protocol>\n\
      <source>\n\
         <name>%s</name>\n\
         <user>%s</user>\n\
         <uuid>%s</uuid>\n\
         <timestamp>%s</timestamp>\n\
         <summary>%s</summary>\n\
      </source>\n\
      <ids>\n\
         <momid>%s</momid>\n\
         <sasid>%s</sasid>\n\
      </ids>\n\
   </header>\n\
   <payload>%s</payload>\n\
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

Message::Message(const std::string &from,
         const std::string &forUser,
         const std::string &summary,
         const std::string &protocol,
         const std::string &protocolVersion,
         const std::string &momid,
         const std::string &sasid) 
{	
  itsQpidMsg.setContent(formatString(LOFAR_MSG_TEMPLATE.c_str(),
        protocol.c_str(), protocolVersion.c_str(), from.c_str(), 
        forUser.c_str(), _uuid().c_str(), _timestamp().c_str(), 
        summary.c_str(), 	momid.c_str(), sasid.c_str(), "%s"));
  itsQpidMsg.setContentType("text/plain");
  itsQpidMsg.setDurable(true);

#ifdef HAVE_LIBXML
  content_as_xml_document = parseXMLString(itsQpidMsg.getContent());
#endif
}

// Read a message from disk (header + payload)
Message::Message(const std::string &rawContent)
{
  itsQpidMsg.setContent(rawContent);

#ifdef HAVE_LIBXML
  content_as_xml_document = parseXMLString(itsQpidMsg.getContent());
#endif
}



Message::Message(const qpid::messaging::Message qpidMsg) 
: 
  itsQpidMsg(qpidMsg)
{
#ifdef HAVE_LIBXML
  content_as_xml_document = parseXMLString(itsQpidMsg.getContent());
#endif
};

Message::~Message()
{
  //xmlFreeDoc(xmlDocPtr);
}

void Message::setXMLPayload (const std::string &payload)
{
  itsQpidMsg.setContent(formatlString(itsQpidMsg.getContent().c_str(),
                                      payload.c_str()));

#ifdef HAVE_LIBXML
  content_as_xml_document = parseXMLString(itsQpidMsg.getContent());
#endif
}

void Message::setTXTPayload (const std::string &payload)
{
  itsQpidMsg.setContent(formatlString(itsQpidMsg.getContent().c_str(), 
                                      payload.c_str()));
#ifdef HAVE_LIBXML
  content_as_xml_document = parseXMLString(itsQpidMsg.getContent());
#endif
}

void Message::setMapPayload (const qpid::types::Variant::Map  &payload)
{
  //TODO: Not implemented
  (void)payload;
}

void Message::setListPayload(const qpid::types::Variant::List &payload)
{
  //TODO: Not implemented
  (void)payload;
}

std::string Message::short_desc() const
{
  return formatString("[%s] %s", uuid().c_str(), summary().c_str());
}

//
// print
//
std::ostream& Message::print (std::ostream& os) const
{
  os << "system         : " << system() << endl;
    os << "systemversion  : " << headerVersion() << endl;
    os << "protocolName   : " << protocol() << endl;
    os << "protocolVersion: " << protocolVersion() << endl;
    os << "summary        : " << summary() << endl;
    os << "timestamp      : " << timestamp() << endl;
    os << "source         : " << from() << endl;
    os << "user           : " << forUser() << endl;
    os << "uuid           : " << uuid() << endl;
    os << "momid          : " << momid() << endl;
    os << "sasid          : " << sasid() << endl;
    os << "payload        : " << payload() << endl;
  os << "BEGIN FULL PACKET" << endl;
  os << itsQpidMsg.getContent() << endl;
  os << "END FULL PACKET" << endl;
  return (os);
}




//
// parseXMLString(tag)
//
xmlDocPtr Message::parseXMLString(const std::string& xml_string)
{
  xmlDocPtr doc;

  // TODO: unicode?
  unsigned length = xml_string.length();

  doc = xmlReadMemory(xml_string.c_str(), length,
    "document", NULL, XML_PARSE_NOBLANKS);


  if (doc == NULL) 
  {
    std::string error_msg = "failed to parse string to xml document";
    LOG_ERROR(error_msg);
    THROW(MessageParseException, error_msg);
  }

  return doc;
}


#ifdef HAVE_LIBXML

xmlNodePtr find_first_element_by_name(xmlNodePtr node, std::string name) 
{
  xmlNodePtr cur_node = NULL;

  // Loop over all the siblings in the 
  for (cur_node = node; cur_node; cur_node = cur_node->next) 
  {
    // If we are at a node
    if (cur_node->type == XML_ELEMENT_NODE) 
    {
      // And it has the correct name
      if (!xmlStrcmp(cur_node->name, (const xmlChar *)name.c_str()))
      {
        return cur_node;
      }
    }
    // Look in the children
    xmlNodePtr child_node = find_first_element_by_name(cur_node->children,
                                                       name);

    // If a child was found
    if (child_node)
    {
      return child_node; 
    }
  }

  // still noting found? return NULL ptr
  return NULL;
}

std::string Message::getXMLvalue(const std::string& key) const
{
  // get the root element
  xmlNodePtr current_node = xmlDocGetRootElement(content_as_xml_document);
  
  cout << "*******************" << endl;
  cout << current_node->content << endl;
  cout << "*******************" << endl;
  vector<std::string>	labels = split(key, '.');
  for(vector<string>::const_iterator label = labels.begin();
    label != labels.end(); ++label)
  {
    //Recursively look in the children for the label
    current_node = find_first_element_by_name(current_node->children,
                                              *label);
    // If not found, raise Exception
    if (!current_node)
    {
      // No child with the correct name was found return NULL
      THROW(MessageParseException, 
        std::string("Could not find tag in xml: ") + *label);
    }
  }
  // If we get here the finding of the correctly named nodes succeeded
  return string((char *)(current_node->content));

}
#else //We dont have HAVE_LIBXML

#warning "Compiling Message without LibXML2 support."
//
// rawParseXMLString(tag)
// Function attempts to retrieve tag(key) from the message.
// THe first entry is returned.
//
std::string Message::getXMLvalue(const std::string& key) const
{
  // get copy of content
  vector<string>	labels = split(key, '.');
  string			content(itsQpidMsg.getContent());

  // loop over subkeys
  string::size_type	offset = 0;
  string::size_type	begin = string::npos;
  string::size_type	end = string::npos;
  string				startTag;


  for (size_t i = 0; i <  labels.size(); ++i) 
  {
    // define tags to find
    startTag = string("<"+labels[i]+">");
    // search begin tag
    begin  = content.find(startTag, offset);
    if (begin == string::npos) 
    {
      THROW(MessageParseException, 
            std::string("Could not find start tag for: '") + key + 
            std::string("' in message header"));
    }
    offset = begin;
  }

  // search end tag
  string stopTag ("</"+labels[labels.size()-1]+">");
  begin+=startTag.size();
  end = content.find(stopTag, begin);
  if (end == string::npos) 
  {
    THROW(MessageParseException,
      std::string("Could not find end tag for: '") + key +
      std::string("' in message header"));
  }
 
  return (content.substr(begin, end - begin));
}
#endif

} // namespace LOFAR
