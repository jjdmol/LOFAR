#ifndef __BGLCO_H__
#define __BGLCO_H__

#define BGL_MEM_SRAM_SIZE (16*1024)
#define BGL_MEM_SRAM_PHYS 0xFFFFC000

#define BGL_MEM_LOCKBOX_SIZE 4096
#define BGL_MEM_LOCKBOX_PHYS 0xD0000000

#define BARRIER_OFFSET(nr) (sizeof(struct BGL_Barrier) * nr)
#define MUTEX_OFFSET(nr) (sizeof(struct BGL_Mutex) * nr)

#define L1_CACHE_LINE_SIZE 32


void bglco_init(void);



/* -------------------------------------------------------------------- */
/*            Lockbox functions                                         */
/* -------------------------------------------------------------------- */

/* A hardware lock used to build mutexes and barriers. This is a
 * hardware-mapped data structure: do not add or remove fields.
 */
typedef struct BGL_Lock {
  volatile unsigned _word0;   /* mutex */
  volatile unsigned _word1;   /* query */
  volatile unsigned _word2;   /* barrier */
  volatile unsigned _word3;
} BGL_Lock;


/* A hardware lock used as a mutex.  This is a hardware-mapped data
 * structure: do not add or remove fields.  Do not access these fields
 * directly, use the BGL_Mutex_XXX() functions below.
 */
typedef struct BGL_Mutex {
  BGL_Lock _lock[1];
} BGL_Mutex;


/* A set of hardware locks used as a barrier.  This is a
 * hardware-mapped data structure: do not add or remove fields. Do not
 * access these fields directly, use the BGL_Barrier_XXX() functions
 * below.
 */
typedef struct BGL_Barrier {
  BGL_Lock _lock[8];
} BGL_Barrier;




/* Ensure that order-of-execution for memory and lockbox accesses is
 * respected.
 */
static inline void BGL_Lock_Msync(void) {
  asm volatile("sync" ::: "memory");
}


/* -------------------------------------------------------------------- */
/*            Mutex operations                                          */
/* -------------------------------------------------------------------- */

/* Acquire a mutex, if it's not currently held by the other processor.
 * Returned: 1=success, 0=failure Hardware side effect: atomically
 * stores a 1 if success
 */
static inline unsigned BGL_Mutex_Try(BGL_Mutex *p) {
  BGL_Lock_Msync();
  unsigned rc = p->_lock[0]._word0 == 0;
  BGL_Lock_Msync();
  return rc;
}

/* Acquire a mutex, spinning if necessary until the other processor
 * releases it. Hardware side effect: atomically stores a 1
 */
static inline void BGL_Mutex_Acquire(BGL_Mutex *p) {
  BGL_Lock_Msync();
  while(p->_lock[0]._word0);
  BGL_Lock_Msync();
}

/* Release a mutex.
 * Hardware side effect: atomically stores a 0
 */
static inline void BGL_Mutex_Release(BGL_Mutex *p) {
  BGL_Lock_Msync();
  p->_lock[0]._word0 = 0;
  BGL_Lock_Msync();
}

/* Is mutex currently locked?
 * Returned: 1=yes 0=no
 */
static inline unsigned BGL_Mutex_Is_Locked(BGL_Mutex *p) {
  return p->_lock[0]._word1;
}


/* -------------------------------------------------------------------- */
/*            Barrier operations                                        */
/* -------------------------------------------------------------------- */

/* Lower other processor's barrier, allowing him to pass through it.
 */
static inline void BGL_Barrier_Lower(BGL_Barrier *p) {
  BGL_Lock_Msync();
  p->_lock[0]._word2 = 0;
  BGL_Lock_Msync();
}

/* Is this processor currently blocked from passing through a barrier?
 * Returned: 1=yes 0=no
 */
static inline unsigned BGL_Barrier_Is_Raised(BGL_Barrier *p) {
  return p->_lock[0]._word2;
}

/* Enter/leave a barrier.
 */
static inline void BGL_Barrier_Pass(BGL_Barrier *p) {
  BGL_Lock_Msync();
  p->_lock[0]._word2 = 0;
  while (p->_lock[0]._word2);
  BGL_Lock_Msync();
}


#endif /* __BGLCO_H__ */


