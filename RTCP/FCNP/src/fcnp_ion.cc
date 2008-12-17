#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sched.h>

#include <bpcore/bgp_collective_inlines.h>
#include <bpcore/bgp_atomic_ops.h>
#include <bpcore/ppc450_inlines.h>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <pthread.h>

#include "fcnp_ion.h"
#include "protocol.h"

#undef USE_TIMER


namespace FCNP_ION {

class Semaphore
{
  public:
    Semaphore(unsigned level = 0)
    :
      level(level)
    {
      pthread_mutex_init(&mutex, 0);
      pthread_cond_init(&increasedLevel, 0);
    }

    ~Semaphore()
    {
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&increasedLevel);
    }

    void up()
    {
      pthread_mutex_lock(&mutex);
      ++ level;
      pthread_cond_signal(&increasedLevel);
      pthread_mutex_unlock(&mutex);
    }

    void down()
    {
      pthread_mutex_lock(&mutex);

      while (level == 0)
	pthread_cond_wait(&increasedLevel, &mutex);

      -- level;
      pthread_mutex_unlock(&mutex);
    }

  private:
    pthread_mutex_t mutex;
    pthread_cond_t  increasedLevel;
    unsigned	    level;
};


class Handshake {
  public:
    struct CnRequest {
      RequestPacket packet __attribute__ ((aligned(16)));
      Semaphore     slotFree, slotFilled;

      CnRequest() : slotFree(1), slotFilled(0) {}
    } cnRequest;

    struct IonRequest {
      size_t	      size;
      char	      *ptr;

      pthread_mutex_t mutex;

      IonRequest()
      {
	pthread_mutex_init(&mutex, 0);
      }

      ~IonRequest()
      {
	pthread_mutex_destroy(&mutex);
      }
    } ionRequest;

    Semaphore writeFinished;

    Handshake() : writeFinished(0) {}
};

static Handshake		handshakes[256][2] __attribute__ ((aligned(16))); // FIXME: variable size
static bool			initialized[256]; // FIXME
static std::vector<Handshake *> scheduledWriteRequests;
static uint32_t			vc0;
static _BGP_Atomic		sendMutex	      = {0};
static pthread_mutex_t		scheduledRequestsLock = PTHREAD_MUTEX_INITIALIZER;


// Reading the tree status words seems to be expensive.  These wrappers
// minimize the number of status word reads.  Do not read/send packets
// without consulting these functions!

static inline void waitForFreeSendSlot()
{
  // only use this function while sendMutex locked!

  static unsigned minimumNumberOfFreeSendFIFOslots;

  while (minimumNumberOfFreeSendFIFOslots == 0) {
    _BGP_TreeFifoStatus stat;

    stat.status_word = _bgp_In32((uint32_t *) (vc0 + _BGP_TRx_Sx));
    minimumNumberOfFreeSendFIFOslots = _BGP_TREE_STATUS_MAX_PKTS - std::max(stat.InjHdrCount, (stat.InjPyldCount + 15) / 16);
  }

  -- minimumNumberOfFreeSendFIFOslots;
}


static unsigned minimumNumberOfFilledReceiveFIFOslots;


static inline void waitForIncomingPacket()
{
  while (minimumNumberOfFilledReceiveFIFOslots == 0) {
    _BGP_TreeFifoStatus stat;

    stat.status_word = _bgp_In32((uint32_t *) (vc0 + _BGP_TRx_Sx));
    minimumNumberOfFilledReceiveFIFOslots = std::min(stat.RecHdrCount, stat.RecPyldCount / 16);
  }

  -- minimumNumberOfFilledReceiveFIFOslots;
}


static inline bool checkForIncomingPacket()
{
  if (minimumNumberOfFilledReceiveFIFOslots == 0) {
    _BGP_TreeFifoStatus stat;

    stat.status_word = _bgp_In32((uint32_t *) (vc0 + _BGP_TRx_Sx));
    minimumNumberOfFilledReceiveFIFOslots = std::min(stat.RecHdrCount, stat.RecPyldCount / 16);

    if (minimumNumberOfFilledReceiveFIFOslots == 0)
      return false;
  }

  -- minimumNumberOfFilledReceiveFIFOslots;
  return true;
}


static void handshakeComplete(Handshake *handshake)
{
  pthread_mutex_lock(&scheduledRequestsLock);
  scheduledWriteRequests.push_back(handshake);
  pthread_mutex_unlock(&scheduledRequestsLock);
}


static inline void sendPacket(_BGP_TreePtpHdr *header, const void *ptr)
{
  //pthread_mutex_lock(&sendMutex);
  while (!_bgp_test_and_set(&sendMutex, 1))
    ;

  waitForFreeSendSlot();
  _bgp_vcX_pkt_inject(&header->word, const_cast<void *>(ptr), vc0);

  //pthread_mutex_unlock(&sendMutex);
  _bgp_msync();
  sendMutex.atom = 0;
}


// Grabbing the sendMutex for each packet is too expensive on the ION.
// Provide a function that grabs one lock for 16 packets.

static inline void send16Packets(_BGP_TreePtpHdr *header, void *ptr)
{
  //pthread_mutex_lock(&sendMutex);
  while (!_bgp_test_and_set(&sendMutex, 1))
    ;

  for (char *p = (char *) ptr, *end = p + 16 * _BGP_TREE_PKT_MAX_BYTES; p < end; p += _BGP_TREE_PKT_MAX_BYTES) {    
    waitForFreeSendSlot();
    _bgp_vcX_pkt_inject(&header->word, p, vc0);
  }

  //pthread_mutex_unlock(&sendMutex);
  _bgp_msync();
  sendMutex.atom = 0;
}


static void sendAck(const RequestPacket *ack)
{
  _BGP_TreePtpHdr header;

  header.Class	   = 0;
  header.Ptp	   = 1;
  header.Irq	   = 1;
  header.PtpTarget = ack->rank;
  header.CsumMode  = _BGP_TREE_CSUM_NONE;

  sendPacket(&header, ack);
}


static void handleRequest(const RequestPacket *request)
{
  Handshake::CnRequest *cnRequest = &handshakes[request->rankInPSet][request->type].cnRequest;

  //std::cout << "handleRequest: rank = " << request->rank << ", core = " << request->core << ", rankInPSet = " << request->rankInPSet << ", type = " << request->type << ", size = " << request->size << std::endl;

  if (request->type == RequestPacket::RESET) {
    if (!initialized[request->rankInPSet]) {
      initialized[request->rankInPSet] = true;
      sendAck(request);
    }
  } else {
    // avoid race conditions between two consecutive requests from the CN
    cnRequest->slotFree.down();
    cnRequest->packet = *request; // TODO: avoid "large" memcpy
    cnRequest->slotFilled.up();
  }
}


static void handleReadRequest(RequestPacket *request, const char *ptr, size_t bytesToGo)
{
  assert(bytesToGo % 16 == 0 && request->size % 16 == 0);

  bytesToGo = request->size = std::min(request->size, bytesToGo);

#if defined USE_TIMER
  unsigned long long start_time = _bgp_GetTimeBase();
  size_t size = bytesToGo;
#endif

  memcpy(request->messageHead, ptr, bytesToGo % _BGP_TREE_PKT_MAX_BYTES);
  ptr += bytesToGo % _BGP_TREE_PKT_MAX_BYTES;
  bytesToGo &= ~(_BGP_TREE_PKT_MAX_BYTES - 1);

  sendAck(request);

  // now send the remaining data, which must be a multiple of the packet size
  assert(bytesToGo % _BGP_TREE_PKT_MAX_BYTES == 0);

  _BGP_TreePtpHdr header;

  header.Class	   = 0;
  header.Ptp	   = 1;
  header.Irq	   = 0;
  header.PtpTarget = request->rank;
  header.CsumMode  = _BGP_TREE_CSUM_NONE;

  const char *end = ptr + bytesToGo;

  for (; ptr < end - 15 * _BGP_TREE_PKT_MAX_BYTES; ptr += 16 * _BGP_TREE_PKT_MAX_BYTES)
    send16Packets(&header, (void *) ptr);

  for (; ptr < end; ptr += _BGP_TREE_PKT_MAX_BYTES)
    sendPacket(&header, (void *) ptr);

#if defined USE_TIMER
  unsigned long long stop_time = _bgp_GetTimeBase();
  std::cout << "read " << size << " bytes to " << request->rankInPSet << " @ " << (8 * size / ((stop_time - start_time) / 850e6) / 1e9) << " Gib/s" << std::endl;
#endif
}


static void handleWriteRequest(RequestPacket *request, char *ptr, size_t bytesToGo)
{
  assert(bytesToGo % 16 == 0 && request->size % 16 == 0);

  bytesToGo = request->size = std::min(request->size, bytesToGo);

#if defined USE_TIMER
  unsigned long long start_time = _bgp_GetTimeBase();
  size_t size = bytesToGo;
#endif

  memcpy(ptr, request->messageHead, bytesToGo % _BGP_TREE_PKT_MAX_BYTES);
  ptr += bytesToGo % _BGP_TREE_PKT_MAX_BYTES;
  bytesToGo &= ~(_BGP_TREE_PKT_MAX_BYTES - 1);

  sendAck(request);

  // now receive the remaining data, which must be a multiple of the packet size
  assert(bytesToGo % _BGP_TREE_PKT_MAX_BYTES == 0);

  for (const char *end = ptr + bytesToGo; ptr < end;) {
    _BGP_TreePtpHdr header;

    waitForIncomingPacket();
    _bgp_vcX_pkt_receive(&header.word, ptr, vc0);

    if (header.Irq)
      handleRequest(reinterpret_cast<RequestPacket *>(ptr));
    else
      ptr += _BGP_TREE_PKT_MAX_BYTES;
  }

#if defined USE_TIMER
  unsigned long long stop_time = _bgp_GetTimeBase();
  std::cout << "write " << size << " bytes from " << request->rankInPSet << " @ " << (8 * size / ((stop_time - start_time) / 850e6) / 1e9) << " Gib/s" << std::endl;
#endif
}


static volatile bool stop;


static void *receive_thread(void *)
{
  RequestPacket request __attribute__((aligned(16)));

  while (!stop) {
    _BGP_TreePtpHdr header;

    if (checkForIncomingPacket()) {
      _bgp_vcX_pkt_receive(&header.word, &request, vc0);
      assert(header.Irq);
      handleRequest(&request);
    }

    pthread_mutex_lock(&scheduledRequestsLock);

    if (scheduledWriteRequests.size() > 0) {
      Handshake *handshake = scheduledWriteRequests.back();
      scheduledWriteRequests.pop_back();
      pthread_mutex_unlock(&scheduledRequestsLock);

      handleWriteRequest(&handshake->cnRequest.packet, handshake->ionRequest.ptr, handshake->ionRequest.size);
      handshake->writeFinished.up();
    } else {
      pthread_mutex_unlock(&scheduledRequestsLock);
    }
  }

  return 0;
}


void IONtoCN_ZeroCopy(unsigned rankInPSet, const void *ptr, size_t size)
{
  assert(size % 16 == 0 && (size_t) ptr % 16 == 0);

  Handshake *handshake = &handshakes[rankInPSet][RequestPacket::ZERO_COPY_READ];
  pthread_mutex_lock(&handshake->ionRequest.mutex);

  while (size > 0) {
    handshake->cnRequest.slotFilled.down();

    // handle all read requests sequentially (and definitely those from multiple
    // cores from the same node!)
    static pthread_mutex_t streamingSendMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&streamingSendMutex);
    handleReadRequest(&handshake->cnRequest.packet, static_cast<const char *>(ptr), size);
    pthread_mutex_unlock(&streamingSendMutex);

    size -= handshake->cnRequest.packet.size;
    ptr = (const void *) ((const char *) ptr + handshake->cnRequest.packet.size);
    handshake->cnRequest.slotFree.up();
  }

  pthread_mutex_unlock(&handshake->ionRequest.mutex);
}


void CNtoION_ZeroCopy(unsigned rankInPSet, void *ptr, size_t size)
{
  assert(size % 16 == 0 && (size_t) ptr % 16 == 0);

  Handshake *handshake = &handshakes[rankInPSet][RequestPacket::ZERO_COPY_WRITE];
  pthread_mutex_lock(&handshake->ionRequest.mutex);

  while (size > 0) {
    handshake->ionRequest.size = size;
    handshake->ionRequest.ptr  = static_cast<char *>(ptr);

    handshake->cnRequest.slotFilled.down();

    pthread_mutex_lock(&scheduledRequestsLock);
    scheduledWriteRequests.push_back(handshake);
    pthread_mutex_unlock(&scheduledRequestsLock);

    handshake->writeFinished.down();
    size -= handshake->cnRequest.packet.size;
    ptr = (void *) ((char *) ptr + handshake->cnRequest.packet.size);

    handshake->cnRequest.slotFree.up();
  }

  pthread_mutex_unlock(&handshake->ionRequest.mutex);
}


#if 0
void writeUnaligned(unsigned rankInPSet, const void *ptr, size_t size)
{
  const char *src = static_cast<const char *>(ptr);

  while (size > 0) {
    size_t chunkSize = size % sizeof buffer;

    memcpy(buffer, src, chunkSize);
    src += chunkSize;
    size -= chunkSize;

    CNtoION_ZeroCopy(rankInPSet, buffer, chunkSize);
  }
}
#endif


static void setAffinity()
{
  cpu_set_t cpu_set;

  CPU_ZERO(&cpu_set);

  for (unsigned cpu = 1; cpu < 4; cpu ++)
    CPU_SET(cpu, &cpu_set);

  if (sched_setaffinity(0, sizeof cpu_set, &cpu_set) != 0) {
    std::cerr << "WARNING: sched_setaffinity failed" << std::endl;
    perror("sched_setaffinity");
  }
}


static void openVC0()
{
  int fd = open("/dev/tree0", O_RDWR);

  if (fd < 0) {
    perror("could not open /dev/tree0");
    exit(1);
  }

  if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
    perror("flock on /dev/tree0");
    exit(1);
  }

  vc0 = (uint32_t) mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (vc0 == (uint32_t) MAP_FAILED) {
    perror("could not mmap /dev/tree0");
    exit(1);
  }
}


static void drainFIFO()
{
  // check if previous run crashed halfway receiving a message

  _BGP_TreeFifoStatus stat;

  // Try to figure out how many quads are lingering around.  This cannot be
  // done 100% reliable (incoming packets do not increase RecHdrCount and
  // RecPyldCount atomically), so accept the answer if it is 16 times the same.

  int quadsToRead, previousQuadsToRead = -1;

  for (unsigned consistentAnswers = 0; consistentAnswers < 16;) {
    stat.status_word = _bgp_In32((uint32_t *) (vc0 + _BGP_TRx_Sx));
    quadsToRead = stat.RecPyldCount - 16 * stat.RecHdrCount;

    if (quadsToRead == previousQuadsToRead) {
      ++ consistentAnswers;
    } else {
      previousQuadsToRead = quadsToRead;
      consistentAnswers = 0;
    }
  }

  if (quadsToRead > 0)
    std::clog << "dropped " << quadsToRead << " lingering quadwords from packets of a previous job" << std::endl;

  while (-- quadsToRead >= 0)
    _bgp_QuadLoad(vc0 + _BGP_TRx_Sx, 0);

  // check if previous run crashed halfway sending a message

  if (stat.InjPyldCount % 16 != 0) {
    // TODO: recover from this
    std::cerr << "previous run crashed while sending a message -- please reboot partition" << std::endl;
    exit(1);
  }

  // drain lingering packets from previous jobs

  uint64_t time    = _bgp_GetTimeBase() + 850000000;
  unsigned dropped = 0;

  while (_bgp_GetTimeBase() < time)
    if (checkForIncomingPacket()) {
      _BGP_TreePtpHdr header;
      _bgp_vcX_hdr_receive(&header.word, vc0);
      _bgp_vcX_pkt_receiveNoHdrNoStore(0, vc0); // drop everything
      ++ dropped;
    }

  if (dropped > 0)
    std::clog << "dropped " << dropped << " lingering packets from previous job" << std::endl;
}


static pthread_t thread;


void init(bool enableInterrupts)
{
  //setAffinity();
  openVC0();
  drainFIFO();

  if (pthread_create(&thread, 0, receive_thread, 0) != 0) {
    perror("pthread_create");
    exit(1);
  }
}


void end()
{
  stop = true;

  if (pthread_join(thread, 0) != 0) {
    perror("pthread_join");
    exit(1);
  }

  close(vc0);
}

} // namespace FCNP_ION
