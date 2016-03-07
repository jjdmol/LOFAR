from threading import Lock, Condition

class MethodTrigger:
  """
    Set a flag when a specific method is called, possibly asynchronously. Caller can wait on this flag.
    
    Example:

      class Foo(object):
        def bar(self):
          pass

      foo = Foo()
      trigger = MethodTrigger(foo, "bar")

      if trigger.wait(): # Waits for 10 seconds for foo.bar() to get called
        print "foo.bar() got called"
      else
        # This will happen, as foo.bar() wasn't called
        print "foo.bar() did not get called"

    Calls that were made before the trigger has been installed will not get recorded.
  """

  def __init__(self, obj, method):
    assert isinstance(obj, object), "Object %s does not derive from object." % (obj,)

    self.obj = obj
    self.method = method
    self.old_func = obj.__getattribute__(method)

    self.called = False
    self.args = []
    self.kwargs = {}

    self.lock = Lock()
    self.cond = Condition(self.lock)

    # Patch the target method
    obj.__setattr__(method, self.trigger)

  def trigger(self, *args, **kwargs):
    # Save the call parameters
    self.args = args
    self.kwargs = kwargs

    # Call the original method
    self.old_func(*args, **kwargs)

    # Restore the original method
    self.obj.__setattr__(self.method, self.old_func)

    # Release waiting thread
    with self.lock:
      self.called = True
      self.cond.notify()

  def wait(self, timeout=10.0):
    # Wait for method to get called
    with self.lock:
      if self.called:
        return True

      self.cond.wait(timeout)

    return self.called
