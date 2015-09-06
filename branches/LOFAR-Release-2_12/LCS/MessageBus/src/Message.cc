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

#include <time.h>

namespace LOFAR {
  using namespace StringUtil;

/*
 * The template for the LOFAR message format.
 */
const string LOFAR_MSG_TEMPLATE = "\
<?xml version=\"1.0\"?>\n\
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
</message>\n\
";

/*
 * Default settings (for this release)
 */
const std::string MessageContent::Defaults::system = "LOFAR";
const std::string MessageContent::Defaults::headerVersion = "1.0.0";

/*
 * Generated settings
 */
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
:
  itsContent(LOFAR_MSG_TEMPLATE)
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

  this->system          = Defaults::system;
  this->headerVersion   = Defaults::headerVersion;

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

MessageContent::MessageContent(const MessageContent &other)
:
  itsContent(other.itsContent)
{
  addProperties();
}

std::string MessageContent::getContent() const
{
  return itsContent.getContent();
}

void MessageContent::addProperties()
{
  system         .attach(&itsContent, "message/header/system");
  headerVersion  .attach(&itsContent, "message/header/version");

  protocol       .attach(&itsContent, "message/header/protocol/name");
  protocolVersion.attach(&itsContent, "message/header/protocol/version");

  name           .attach(&itsContent, "message/header/source/name");
  user           .attach(&itsContent, "message/header/source/user");
  uuid           .attach(&itsContent, "message/header/source/uuid");

  summary        .attach(&itsContent, "message/header/source/summary");
  timestamp      .attach(&itsContent, "message/header/source/timestamp");

  momid          .attach(&itsContent, "message/header/ids/momid");
  sasid          .attach(&itsContent, "message/header/ids/sasid");

  payload        .attach(&itsContent, "message/payload");
  header         .attach(&itsContent, "message/header");
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
  itsContent.insertXML("message/payload", payload);
}

void MessageContent::setTXTPayload (const std::string         &payload)
{
  itsContent.setXMLvalue("message/payload", payload);
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

Message::Message(const MessageContent &content)
:
  itsQpidMsg(content.qpidMsg())
{
}


} // namespace LOFAR
