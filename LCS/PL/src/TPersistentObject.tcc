#include <PL/TPersistentObject.h>


namespace LCS
{
  namespace PL
  {


    template<typename T>
    inline void TPersistentObject<T>::insert(const PersistenceBroker* const b)
    {
      itsOid.get();    // This effectively initializes itsOid
      insert(itsOid);
    }


    template<typename T>
    inline void TPersistentObject<T>::update(const PersistenceBroker* const b)
    {
      update(itsOid);
    }


    template<typename T>
    inline void TPersistentObject<T>::save(const PersistenceBroker* const b)
    {
      if (isPersistent()) {
	  update(b);
      }
      else {
	  insert(b);
      }
      isPersistent(true);
    }


  } // namespace PL

} // namespace LCS
