//#  Copyright (C) 2015
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
#include <UnitTest++.h>
#include <MessageBus/Message.h>

#include <iostream>

using namespace LOFAR;
using namespace std;

SUITE(MessageContent) {
  TEST(default) {
    MessageContent msg;
  }

  TEST(newmsg) {
    // Create a message from scratch
    MessageContent msg("NAME", "USER", "SUMMARY", "PROTOCOL", "1.2", "MOMID", "SASID");

    CHECK_EQUAL("NAME",     msg.name.get());
    CHECK_EQUAL("USER",     msg.user.get());
    CHECK_EQUAL("SUMMARY",  msg.summary.get());
    CHECK_EQUAL("PROTOCOL", msg.protocol.get());
    CHECK_EQUAL("1.2",      msg.protocolVersion.get());
    CHECK_EQUAL("MOMID",    msg.momid.get());
    CHECK_EQUAL("SASID",    msg.sasid.get());

    std::cout << msg << std::endl;
    std::cout << msg.qpidMsg().getContent() << std::endl;
  }

  TEST(existingmsg) {
    MessageContent orig("NAME", "USER", "SUMMARY", "PROTOCOL", "1.2", "MOMID", "SASID");

    // Create a qpid message and parse it again
    MessageContent copy(orig.qpidMsg());

    CHECK_EQUAL(orig.name,            copy.name);
    CHECK_EQUAL(orig.user,            copy.user);
    CHECK_EQUAL(orig.summary,         copy.summary);
    CHECK_EQUAL(orig.protocol,        copy.protocol);
    CHECK_EQUAL(orig.protocolVersion, copy.protocolVersion);
    CHECK_EQUAL(orig.momid,           copy.momid);
    CHECK_EQUAL(orig.sasid,           copy.sasid);
  }

  TEST(modifymsg) {
    MessageContent orig("NAME", "USER", "SUMMARY", "PROTOCOL", "1.2", "MOMID", "SASID");

    orig.protocolVersion = "1.3";

    CHECK_EQUAL("1.3", orig.protocolVersion.get());
  }
}

int main() {
  INIT_LOGGER("tMessage");

  return UnitTest::RunAllTests() > 0;
}
