import unittest
from lofar.common.methodtrigger import MethodTrigger

from threading import Thread
import time

class TestMethodTrigger(unittest.TestCase):
  def setUp(self):
    # Create a basic object
    class TestClass(object):
      def func(self):
        pass

    self.testobj = TestClass()

    # Install trigger
    self.trigger = MethodTrigger(self.testobj, "func")

  def test_no_call(self):
    """ Do not trigger. """

    # Wait for trigger
    self.assertFalse(self.trigger.wait(0.1))

  def test_serial_call(self):
    """ Trigger and wait serially. """

    # Call function
    self.testobj.func()

    # Wait for trigger
    self.assertTrue(self.trigger.wait(0.1))

  def test_parallel_call(self):
    """ Trigger and wait in parallel. """

    class wait_thread(Thread):
      def __init__(self, trigger):
        Thread.__init__(self)
        self.result = None
        self.trigger = trigger

      def run(self):
        self.result = self.trigger.wait(1.0)

    class call_thread(Thread):
      def __init__(self,func):
        Thread.__init__(self)
        self.func = func

      def run(self):
        time.sleep(0.5)
        self.func()

    # Start threads
    t1 = wait_thread(self.trigger)
    t1.start()
    t2 = call_thread(self.testobj.func)
    t2.start()

    # Wait for them to finish
    t1.join()
    t2.join()

    # Inspect result
    self.assertTrue(t1.result)

class TestArgs(unittest.TestCase):
  def setUp(self):
    # Create a basic object
    class TestClass(object):
      def func(self, a, b, c=None, d=None):
        pass

    self.testobj = TestClass()

    # Install trigger
    self.trigger = MethodTrigger(self.testobj, "func")

  def test_args(self):
    """ Trigger and check args. """

    # Call function
    self.testobj.func(1, 2)

    # Wait for trigger
    self.assertTrue(self.trigger.wait(0.1))

    # Check stored arguments
    self.assertEqual(self.trigger.args, (1, 2))

  def test_kwargs(self):
    """ Trigger and check kwargs. """

    # Call function
    self.testobj.func(a=1, b=2)

    # Wait for trigger
    self.assertTrue(self.trigger.wait(0.1))

    # Check stored arguments
    self.assertEqual(self.trigger.kwargs, {"a": 1, "b": 2})

  def test_full(self):
    """ Trigger and check both args and kwargs. """

    # Call function
    self.testobj.func(1, 2, c=3, d=4)

    # Wait for trigger
    self.assertTrue(self.trigger.wait(0.1))

    # Check stored arguments
    self.assertEqual(self.trigger.args, (1, 2))
    self.assertEqual(self.trigger.kwargs, {"c": 3, "d": 4})

def main(argv):
  unittest.main(verbosity=2)

if __name__ == "__main__":
  # run all tests
  import sys
  main(sys.argv[1:])

