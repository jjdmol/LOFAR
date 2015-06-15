//#  MessageContent.cc: one_line_description
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
#include <MessageBus/Message.h>

#ifdef HAVE_QPID
#include <qpid/types/Uuid.h>
#endif

#include <time.h>


namespace LOFAR {
  using namespace StringUtil;

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
  addProperties();
}

MessageContent::MessageContent(const std::string &from,
				 const std::string &forUser,
				 const std::string &summary,
				 const std::string &protocol,
				 const std::string &protocolVersion,
				 const std::string &momid,
				 const std::string &sasid)
:
  itsContent(LOFAR_MSG_TEMPLATE)
{
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
:
  itsContent(qpidMsg.getContent())
{
  addProperties();
}

MessageContent::~MessageContent()
{
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

  qpidMsg.setContent(itsContent);
  qpidMsg.setContentType("text/plain");
  qpidMsg.setDurable(true);

  return qpidMsg;
}

void MessageContent::setXMLPayload (const std::string         &payload)
{
  setXMLvalue("message/payload", payload);
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
    os << "system         : " << system << endl;
    os << "systemversion  : " << headerVersion << endl;
    os << "protocolName   : " << protocol << endl;
    os << "protocolVersion: " << protocolVersion << endl;
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
	// get copy of content
	vector<string>	labels = split(key, '/');

	// loop over subkeys
	string::size_type	offset = 0;
	string::size_type	begin = string::npos;
	string::size_type	end = string::npos;
	string				startTag;
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
}

void MessageContent::setXMLvalue(const string& key, const string &data)
{
	// get copy of content
	vector<string>	labels = split(key, '/');

	// loop over subkeys
	string::size_type	offset = 0;
	string::size_type	begin = string::npos;
	string::size_type	end = string::npos;
	string				startTag;
	for (size_t i = 0; i <  labels.size(); ++i) {
		// define tags to find
		startTag = string("<"+labels[i]+">");
		// search begin tag
		begin  = itsContent.find(startTag, offset);
		if (begin == string::npos) {
			return;
		}
		offset = begin;
	}
	// search end tag
	string stopTag ("</"+labels[labels.size()-1]+">");
	begin+=startTag.size();
	end = itsContent.find(stopTag, begin);
	if (end == string::npos) {
		return;
	}

	itsContent.replace(begin, end - begin, data);
}

Message::Message(const MessageContent &content)
:
  itsQpidMsg(content.qpidMsg())
{
}


} // namespace LOFAR
