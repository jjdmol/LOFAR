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
#include <Common/Exception.h>
#include <Common/Exceptions.h>
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
   <payload>\n\
%s\n\
   </payload>\n\
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
:
  content_as_xml_tree(NULL),
  xml_content_parsed(false)
{	
	itsQpidMsg.setContent(formatString(LOFAR_MSG_TEMPLATE.c_str(),
        protocol.c_str(), protocolVersion.c_str(), from.c_str(), 
        forUser.c_str(), _uuid().c_str(), _timestamp().c_str(), 
        summary.c_str(), 	momid.c_str(), sasid.c_str(), "%s"));
  itsQpidMsg.setContentType("text/plain");
  itsQpidMsg.setDurable(true);
}

// Read a message from disk (header + payload)
Message::Message(const std::string &rawContent)
:
  content_as_xml_tree(NULL),
  xml_content_parsed(false)
{
	itsQpidMsg.setContent(rawContent);
}

Message::~Message()
{
  //xmlFreeDoc(xmlDocPtr);
}

void Message::setXMLPayload (const std::string &payload)
{
	itsQpidMsg.setContent(formatlString(itsQpidMsg.getContent().c_str(),
                                      payload.c_str()));
}

void Message::setTXTPayload (const std::string &payload)
{
	itsQpidMsg.setContent(formatlString(itsQpidMsg.getContent().c_str(), 
                                      payload.c_str()));
}

void Message::setMapPayload (const qpid::types::Variant::Map  &payload)
{
  (void)payload;
}

void Message::setListPayload(const qpid::types::Variant::List &payload)
{
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
                      "document", NULL, 0);
  if (doc == NULL) 
  {
    std::string error_msg = "failed to parse string to xml document";
    LOG_ERROR(error_msg);
    THROW(Exception, error_msg);
  }

  return doc;
}




//
// getXMLvalue(tag)
//
string Message::getXMLvalue(const string& key) const
{


	// get copy of content
	vector<string>	labels = split(key, '.');
	string			content(itsQpidMsg.getContent());

	// loop over subkeys
	string::size_type	offset = 0;
	string::size_type	begin = string::npos;
	string::size_type	end = string::npos;
	string				startTag;
	for (size_t i = 0; i <  labels.size(); ++i) {
		// define tags to find
		startTag = string("<"+labels[i]+">");
		// search begin tag
		begin  = content.find(startTag, offset);
		if (begin == string::npos) {
			return ("???");
		}
		offset = begin;
	}
	// search end tag
	string stopTag ("</"+labels[labels.size()-1]+">");
	begin+=startTag.size();
	end = content.find(stopTag, begin);
	if (end == string::npos) {
		return ("???");
	}
 
	return (content.substr(begin, end - begin));
}


} // namespace LOFAR
