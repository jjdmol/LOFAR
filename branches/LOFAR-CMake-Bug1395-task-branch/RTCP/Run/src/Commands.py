import os
import threading
import time



class Command(object):
    """
    Executes an external command
    """

    def __init__(self, commandstr):
        self.commandstr = commandstr
        self.done = False
        self.success = False
    def isDone(self):
        return self.done
    def waitForDone(self):
        raise Exception('Waiting for done on the Command Base class')
    def isSuccess(self):
        return self.success


class ExtCommand(Command):
    """
    Executes an external command immediately
    """

    def __init__(self, commandstr):
        Command.__init__(self, commandstr)
        print('Immediately executing ' + self.commandstr)
        if os.system(self.commandstr) == 0:
            self.success = True
        else:
            self.success = False
        self.done = True


      

class AsyncThreadCommand(Command):
    """
    Executes an external command asynchronously using threads
    """

    class CommandThread(threading.Thread):
        def __init__(self, commandstr):
            threading.Thread.__init__(self)
            self.commandstr = commandstr
            self.lock = threading.Lock()
            self.done = False
            self.success = False

        def run(self):
            result = (os.system(self.commandstr) == 0)
           
            self.lock.acquire()
            self.done = True
            self.success = result
            self.lock.release()

        def isDone(self):
            self.lock.acquire()
            done = self.done
            self.lock.release()
            return done

        def isSuccess(self):
            self.lock.acquire()
            success = self.success
            self.lock.release()
            return success
            
    def __init__(self, commandstr, timeout):
        Command.__init__(self, commandstr)
        self.thread = AsyncThreadCommand.CommandThread(commandstr)
        print('Threaded executing ' + self.commandstr)
        self.thread.start()
        self.startTimeOfRun = time.time()
        self.timeout = timeout

    def isTimedOut(self):
        return (self.timeout and (time.time() - self.startTimeOfRun > self.timeout))

    def isDone(self):
        return self.thread.isDone()

    def waitForDone(self):
        while not self.thread.isDone():
            time.sleep(2)

    def isSuccess(self):
        if not self.thread.isDone():
            self.waitForDone()
        return self.thread.isSuccess()

    def abort(self):
        print('Cannot stop asynchronous commands (yet)')
        
