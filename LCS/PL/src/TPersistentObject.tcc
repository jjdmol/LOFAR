#include <PL/TPersistentObject.h>

namespace LCS
{
  namespace PL
  {

    template<typename T>
    inline void TPersistentObject<T>::insert(const PersistenceBroker* const b)
    {
      // Insert implies that a new object must be inserted into the database.
      // So, we need to have a new ObjectId.
    }

    template<typename T>
    inline void TPersistentObject<T>::save(const PersistenceBroker* const b)
    {
      if (isPersistent) {
	update(b);
      }
      else {
	insert(b);
      }
    }

  } // namespace PL

} // namespace LCS
