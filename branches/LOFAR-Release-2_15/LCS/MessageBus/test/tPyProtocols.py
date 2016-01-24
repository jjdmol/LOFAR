#!/usr/bin/env python
from lofar.parameterset import parameterset

# Test task.feedback.dataproducts
from lofar.messagebus.protocols import TaskFeedbackDataproducts

parset = parameterset()
parset.add("foo", "bar")

msg = TaskFeedbackDataproducts(
  "from",
  "forUser",
  "summary",
  1,
  2,
  parset)

# Test task.feedback.processing
from lofar.messagebus.protocols import TaskFeedbackProcessing

parset = parameterset()
parset.add("foo", "bar")

msg = TaskFeedbackProcessing(
  "from",
  "forUser",
  "summary",
  1,
  2,
  parset)

# Test task.feedback.state
from lofar.messagebus.protocols import TaskFeedbackState

msg = TaskFeedbackState(
  "from",
  "forUser",
  "summary",
  1,
  2,
  True)

