#include <GCF/Thread.h>

#include <Common/CheckConfig.h>
#ifdef USE_THREADS
CHECK_CONFIG_CC(UseThreads,yes);
#else
CHECK_CONFIG_CC(UseThreads,no);
#endif

namespace LOFAR
{
  namespace GCF 
  {
    namespace Thread 
    {
#ifdef USE_THREADS
  void * dummy_pvoid;
  int dummy_int;
  const Attributes _null_attributes;

  const Attributes & Attributes::Null ()
  { return _null_attributes; }
        
  Attributes joinable ()
  { return Attributes(Attributes::JOINABLE); }

  Attributes detached ()
  { return Attributes(Attributes::DETACHED); }

  // -----------------------------------------------------------------------
  // Thread functions
  // -----------------------------------------------------------------------

  // returns thread id of self
  ThrID self ()
  { 
    return ThrID::self(); 
  }

  //  create creates a thread
  ThrID create (void * (*start)(void*), void* arg, const Attributes& attr)
  { 
    pthread_t id = 0;
    pthread_create(&id, attr, start, arg);
    return ThrID(id);
  }
  
  // Exits a thread
  void exit (void* value)
  { 
    pthread_exit(value); 
  }

  // Sets the sigmask for a thread
  int signalMask (int how, const sigset_t* newmask, sigset_t* oldmask)
  {
    return pthread_sigmask(how, newmask, oldmask);
  }

  // Sets a single signal in a thread's sigmask
  int signalMask (int how, int sig, sigset_t* oldmask)
  {
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, sig);
    return pthread_sigmask(how, &sset, oldmask);
  }

  // sets the thread's cancellation state
  int setCancelState (int state, int& oldstate)
  {
    return pthread_setcancelstate(state, &oldstate);
  }

  // sets the thread's cancellation type
  int setCancelType (int type, int& oldtype)
  {
    return pthread_setcanceltype(type, &oldtype);
  }

  void testCancel ()
  {
    pthread_testcancel();
  }

    // Class Thread::ThrID 

    // Additional Declarations
#endif
    } // namespace Thread
  } // namespace GCF
} // namespace LOFAR
