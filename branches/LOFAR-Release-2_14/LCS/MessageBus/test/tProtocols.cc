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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <UnitTest++.h>
#include <MessageBus/Protocols/TaskFeedbackDataproducts.h>
#include <MessageBus/Protocols/TaskFeedbackProcessing.h>
#include <MessageBus/Protocols/TaskFeedbackState.h>
#include <MessageBus/Protocols/TaskSpecificationSystem.h>

#include <iostream>

using namespace LOFAR;
using namespace LOFAR::Protocols;
using namespace std;

TEST(TaskFeedbackDataproducts) {
  // Create some payload
  ParameterSet feedback;
  feedback.add("foo", "bar");

  // Construct message
  TaskFeedbackDataproducts tfdp( "FROM", "FORUSER", "SUMMARY", "MOMID", "SASID", feedback );

  // Verify payload
  ParameterSet parsedFeedback = tfdp.feedback();

  CHECK_EQUAL("bar", parsedFeedback.getString("foo", ""));
}

TEST(TaskFeedbackProcessing) {
  // Create some payload
  ParameterSet feedback;
  feedback.add("foo", "bar");

  // Construct message
  TaskFeedbackProcessing tfp( "FROM", "FORUSER", "SUMMARY", "MOMID", "SASID", feedback );

  // Verify payload
  ParameterSet parsedFeedback = tfp.feedback();

  CHECK_EQUAL("bar", parsedFeedback.getString("foo", ""));
}

SUITE(TaskFeedbackState) {
  TEST(Finished) {
    // Construct message
    TaskFeedbackState tfs( "FROM", "FORUSER", "SUMMARY", "MOMID", "SASID", true );

    // Verify payload
    CHECK_EQUAL("finished", tfs.state.get());
  }

  TEST(Aborted) {
    // Construct message
    TaskFeedbackState tfs( "FROM", "FORUSER", "SUMMARY", "MOMID", "SASID", false );

    // Verify payload
    CHECK_EQUAL("aborted", tfs.state.get());
  }
}

TEST(TaskSpecificationSystem) {
  // Create some payload
  ParameterSet specs;
  specs.add("foo", "bar");

  // Construct message
  TaskSpecificationSystem tfss( "FROM", "FORUSER", "SUMMARY", "MOMID", "SASID", specs );

  // Verify payload
  ParameterSet parsedSpecs = tfss.specifications();

  CHECK_EQUAL("bar", parsedSpecs.getString("foo", ""));
}

int main() {
  INIT_LOGGER("tMessage");

  return UnitTest::RunAllTests() > 0;
}
