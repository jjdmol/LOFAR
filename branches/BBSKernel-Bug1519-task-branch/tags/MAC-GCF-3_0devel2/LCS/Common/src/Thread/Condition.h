#ifndef Condition_h
#define Condition_h 1

// Mutex
#include "Common/Thread/Mutex.h"

namespace Thread {
#ifdef USE_THREADS

  //##ModelId=3D1049B50012
  class Condition : public Mutex
  {
    public:
    //##ModelId=3D10B9920176
        Condition();

        //##ModelId=3DB935A200EF
        Condition (const Condition &);

    //##ModelId=3DB935A20153
        ~Condition();


        //##ModelId=3D10BC84006D
        Condition & operator = (const Condition &);

        //##ModelId=63F66C39FEED
        int broadcast () const;

        //##ModelId=8892C440FEED
        int signal () const;

        //##ModelId=47B70187FEED
        int wait () const;

        //##ModelId=30EB4F86FEED
        int wait (int sec, int ns) const;

        //##ModelId=D33D2F2EFEED
        int wait (double sec) const;

      // Additional Public Declarations
    //##ModelId=3DB935A20185
        const char * debug (int = 1,const string & = "",const char * = 0 ) const
        { return "Condition"; }
    //##ModelId=3DB935A202F8
        string sdebug (int = 1,const string & = "",const char * = 0 ) const
        { return debug(); }
    private:
      // Data Members for Class Attributes

        //##ModelId=3D1058FA025A
        mutable pthread_cond_t cond;

  };

  // Class Thread::Condition 

//##ModelId=3D10B9920176
  inline Condition::Condition()
      : Mutex(RECURSIVE)
  {
    pthread_cond_init(&cond,NULL);
  }

//##ModelId=3DB935A200EF
  inline Condition::Condition (const Condition &)
      : Mutex(RECURSIVE)
  {
    pthread_cond_init(&cond,NULL);
  }


//##ModelId=3DB935A20153
  inline Condition::~Condition()
  {
    pthread_cond_destroy(&cond);
  }



//##ModelId=3D10BC84006D
  inline Condition & Condition::operator = (const Condition &)
  {
    return *this;
  }

//##ModelId=63F66C39FEED
  inline int Condition::broadcast () const
  {
    return pthread_cond_broadcast(&cond); 
  }

//##ModelId=8892C440FEED
  inline int Condition::signal () const
  {
    return pthread_cond_signal(&cond); 
  }

//##ModelId=47B70187FEED
  inline int Condition::wait () const
  {
    return pthread_cond_wait(&cond,&mutex); 
  }

//##ModelId=30EB4F86FEED
  inline int Condition::wait (int sec, int ns) const
  {
    struct timespec ts = { sec,ns };
    return pthread_cond_timedwait(&cond,&mutex,&ts); 
  }

//##ModelId=D33D2F2EFEED
  inline int Condition::wait (double sec) const
  {
    int isec = static_cast<int>(sec);
    return wait(isec,static_cast<int>((sec-isec)*1e+9)); 
  }

  // Class Thread::Condition 

#else
  class Condition : public Mutex 
  {
    public:
        Condition() {};
        Condition (const Condition &) {};
        int broadcast () const { return 0; };
        int signal () const { return 0; };
        int wait () const { return 0; };
        int wait (int, int) const { return 0; };
        int wait (double) { return 0; };
        
        const char * debug (int = 1,const string & = "",const char * = 0 ) const
        { return ""; }
        string sdebug (int = 1,const string & = "",const char * = 0 ) const
        { return debug(); }
  };
#endif

} // namespace Thread


#endif
