#ifndef Key_h
#define Key_h 1

// Thread
#include "Common/Thread/Thread.h"

namespace Thread {
#ifdef USE_THREADS

  //##ModelId=3D1049B402A5
  class Key 
  {
    public:
      //##ModelId=3DB935A10284
      typedef void* Data;
        //##ModelId=5984F88AFEED
        Key (void (*destructor) (void*) = 0);

    //##ModelId=3DB935A300E6
        ~Key();


        //##ModelId=3B3DE74FFEED
        Key::Data get () const;

        //##ModelId=60738D38FEED
        int set (Key::Data value);

        //##ModelId=AFEDB0DBFEED
        operator Key::Data () const;

        //##ModelId=66708D53FEED
        Thread::Key& operator = (Key::Data value);

    private:
    //##ModelId=3DB935A3012D
        Key(const Key &right);

    //##ModelId=3DB935A301AF
        Key & operator=(const Key &right);

    private:
      // Data Members for Class Attributes

        //##ModelId=3D1060CB03CF
        pthread_key_t key;

  };

  // Class Thread::Key 

//##ModelId=5984F88AFEED
//##ModelId=3DB935A3012D
  inline Key::Key (void (*destructor) (void*))
  {
    pthread_key_create(&key,destructor); 
  }


//##ModelId=3DB935A300E6
  inline Key::~Key()
  {
    pthread_key_delete(key); 
  }



//##ModelId=3B3DE74FFEED
  inline Key::Data Key::get () const
  {
    return pthread_getspecific(key); 
  }

//##ModelId=60738D38FEED
  inline int Key::set (Key::Data value)
  {
    return pthread_setspecific(key,value); 
  }

  inline Key::operator Key::Data () const
  {
    return get();
  }

//##ModelId=66708D53FEED
  inline Thread::Key& Key::operator = (Key::Data value)
  {
    set(value);
    return *this;
  }

  // Class Thread::Key 

#else
  class Key 
  { 
    public:
        typedef void * Data; 
  };
#endif

} // namespace Thread


#endif
