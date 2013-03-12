#if !defined OPEN_MP_SUPPORT_H
#define OPEN_MP_SUPPORT_H

#include <omp.h>


class OMP_Lock
{
public:
  OMP_Lock()
  {
    omp_init_lock(&omp_lock);
  }

  ~OMP_Lock()
  {
    omp_destroy_lock(&omp_lock);
  }

  void lock()
  {
    omp_set_lock(&omp_lock);
  }

  void unlock()
  {
    omp_unset_lock(&omp_lock);
  }

private:
  omp_lock_t omp_lock;
};


class OMP_ScopedLock
{
public:
  OMP_ScopedLock(OMP_Lock &omp_lock)
    :
    omp_lock(omp_lock)
  {
    omp_lock.lock();
  }

  ~OMP_ScopedLock()
  {
    omp_lock.unlock();
  }

private:
  OMP_Lock &omp_lock;
};

#endif
