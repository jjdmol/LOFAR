#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <time.h>
#include <netdb.h>
#include <assert.h>

/* max size of a packet we support */
#define MAX_PACKETSIZE		8960

/* sizes of headers are specified seperately since sizeof() may add padding */

#define IP_HEADERSIZE		20
struct IP_header {
                               /* expected or typical value (in hex) */
  unsigned version:         4; /* 4 */
  unsigned headerlength:    4; /* 5 or 6 (# of 32-bit words)*/
  unsigned:                 8; /* 00 (differentiated services) */
  unsigned totallength:    16; /* 1f30 = 7984 decimal */
  unsigned:                16; /* 0000 (identification) */
  unsigned:                 1; /* 0 (reserved flag) */
  unsigned dontfragment:    1; /* 1 */
  unsigned morefragments:   1; /* 0 */
  unsigned:                13; /* 0 (fragment offset) */
  unsigned ttl:             8; /* 80 */
  unsigned protocol:        8; /* 11 = UDP */
  unsigned headercrc:      16;
  unsigned char sourceIP[4];
  unsigned char destIP[4];
};

#define UDP_HEADERSIZE		8
struct UDP_header {
  uint16_t sourcePort; /* 10fa = 4346d or higher */
  uint16_t destPort;   /* = sourcePort */
  uint16_t length;     /* = 8 * #beamlets * #times + 16 */
  uint16_t crc;        /* 0000 = disabled */
};

/* see also RTCP/IONProc/src/RSP.h */
#define EPA_HEADERSIZE		16
struct EPA_header {
  /* little endian! */
  uint16_t version;
  uint16_t configuration;
  uint16_t station;
  unsigned char beamlets;    /* 62 */
  unsigned char times;       /* 16 */
  time_t RSPtimestamp;       /* time() in UTC */
  uint32_t blockSequenceNumber;
};

unsigned char packet[MAX_PACKETSIZE];

/* shortcuts for various headers */
#define IP			(*(struct IP_header *)packet)
#define UDP			(*(struct UDP_header *)((char*)packet + IP.headerlength * 4))
#define EPA			(*(struct EPA_header *)((char*)&UDP + UDP_HEADERSIZE))
#define DATA			((signed char *)((char*)&EPA + EPA_HEADERSIZE))

#define PAYLOAD_SIZE		(UDP.length - EPA_HEADERSIZE)
#define EXPECTED_PPS(clock)	(1.0*clock*1e6/1024/EPA.times)

/* a filter for the packets we're looking for */
#define PACKETFILTER(port)	(IP.protocol == 0x11 && UDP.destPort == port)

int create_socket( int port )
{
  struct sockaddr_in sa;
  int		     sk;
  struct hostent     *host;

  if ( !(host = gethostbyname("0")) ) {
    perror("gethostbyname");
    exit(1);
  }
  
  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_port   = htons(port);
  memcpy(&sa.sin_addr, host->h_addr, host->h_length);

  if ((sk = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
    perror("socket");
    exit(1);
  }

  if (bind(sk, (struct sockaddr *) &sa, sizeof sa) < 0) {
    perror("bind");
    exit(1);
  }

  return sk;
}

/* crude, but works on BG/P */
uint32_t hostname()
{
  char name[255];
  struct hostent *host;

  gethostname(name,sizeof name);
  host=gethostbyname(name);

  return ((struct in_addr*)host->h_addr)->s_addr;
}

void set_affinity( char *cpus )
{
  cpu_set_t cpu_set;
  unsigned  cpu;

  CPU_ZERO(&cpu_set);

  for (cpu = 0; cpu < strlen(cpus); cpu ++)
    CPU_SET(cpus[cpu]-'0', &cpu_set);

  if (sched_setaffinity(0, sizeof cpu_set, &cpu_set) != 0)
    perror("sched_setaffinity");
}

float get_packetrate( int fd, int port, float seconds ) {
  struct timeval tv;
  unsigned long now,end_time;
  unsigned nr;

  nr = 0;
  gettimeofday( &tv, NULL );
  now = tv.tv_sec * 1000000 + tv.tv_usec;
  end_time = now + seconds * 1000000;

  tv.tv_sec = seconds;
  tv.tv_usec = 0;

  while( 1 ) {
    int retval;
    fd_set rfds;
    char buf[MAX_PACKETSIZE];

    FD_ZERO( &rfds );
    FD_SET( fd, &rfds );

    gettimeofday( &tv, NULL );
    now = tv.tv_sec * 1000000 + tv.tv_usec;
    if( now >= end_time ) {
      break;
    }

    tv.tv_sec = (end_time-now) / 1000000;
    tv.tv_usec = (end_time-now) % 1000000;
    retval = select( fd+1, &rfds, NULL, NULL, &tv );

    if( retval == -1 ) {
      perror("select");
      exit(1);
    } else if( retval ) {
      recv( fd, &packet, sizeof buf, 0 );
      if( PACKETFILTER( port ) ) {
        nr++;
      }
    } else {
      break;
    }
  }

  return 1.0*nr/seconds;
}

void swap32( char *nr ) {
  char tmp;

  tmp = nr[0];
  nr[0] = nr[3];
  nr[3] = tmp;

  tmp = nr[1];
  nr[1] = nr[2];
  nr[2] = tmp;
}

void swap16( char *nr ) {
  char tmp;

  tmp = nr[0];
  nr[0] = nr[1];
  nr[1] = tmp;
}


int main( int argc, char **argv ) {
  int fd;
  int port;
  float rate;

  /* weird things happen if RSPtimestamp is not the same size as the input
     for localtime() */
  assert( sizeof(time_t)==4 );

  if( argc < 2 ) {
    printf("Outputs the number of UDP packets received per second.\n\nUsage: %s port\n", argv[0] );
    exit(1);
  }

  port = atoi( argv[1] );

  /* avoid core 0 since it handles the massive number of eth0 interrupts */
  set_affinity( "123" );

  fd = create_socket( port );

  rate = get_packetrate( fd, atoi( argv[1] ), 0.5 );
  if( rate < 1.0 ) {
    printf("NOK Packet rate:        %.2f pps\n",rate);
  } else {
    int zeros = 0;
    int i;

    do {
      recv( fd, &packet, sizeof packet, 0 );
    } while( !PACKETFILTER( port ) );

    /* little endian -> big endian */
    swap16( (char*)&EPA.station );
    swap32( (char*)&EPA.RSPtimestamp );

    for( i = 0; i < PAYLOAD_SIZE; i++ ) {
      if( !DATA[i] ) {
        zeros++;
      }
    }

    union {
      uint32_t integer;
      unsigned char parts[4];
    } myip;
    myip.integer = hostname();

    int destipok = 1;

    for( i = 0; i < 4; i++ ) {
      if( myip.parts[i] != IP.destIP[i] ) {
        destipok = 0;
      }
    }

    /*printf("Source:                  %d.%d.%d.%d:%d\n",IP.sourceIP[0],IP.sourceIP[1],IP.sourceIP[2],IP.sourceIP[3],UDP.sourcePort);*/

    if( destipok ) {
      printf(" OK Dest:                %d.%d.%d.%d:%d\n",IP.destIP[0],IP.destIP[1],IP.destIP[2],IP.destIP[3],UDP.destPort);
    } else {
      printf("NOK Dest:                %d.%d.%d.%d:%d (my ip: %d.%d.%d.%d)\n",IP.destIP[0],IP.destIP[1],IP.destIP[2],IP.destIP[3],UDP.destPort,myip.parts[0],myip.parts[1],myip.parts[2],myip.parts[3]);
    }

    if( EPA.beamlets == 62 ) {
      printf(" OK Beamlets:            %d\n",EPA.beamlets);
    } else {
      printf("NOK Beamlets:            %d (should be 62?)\n",EPA.beamlets);
    }

    if( EPA.times == 16 ) {
      printf(" OK Time samples:        %d\n",EPA.times);
    } else {
      printf("NOK Time samples:        %d (should be 16?)\n",EPA.times);
    }

    if( EPA.RSPtimestamp == -1 ) {
      printf("NOK Timestamp UTC:       UNDEFINED (0xffffffff)\n");
    } else {
      time_t offset = time(NULL) - EPA.RSPtimestamp;
      if( offset < -5 || offset > 5 ) {
        printf("NOK Timestamp UTC:       %s",asctime(localtime(&EPA.RSPtimestamp)));
      } else {
        printf(" OK Timestamp UTC:       %s",asctime(localtime(&EPA.RSPtimestamp)));
      }
    }

    if( zeros == PAYLOAD_SIZE ) {
      printf("NOK Zeros in payload:    %.f %%\n",100.0*zeros/PAYLOAD_SIZE);
    } else if( zeros > 0.5 * PAYLOAD_SIZE ) {
      printf("NOK Zeros in payload:    %.f %% (did you set up %d beamlets?)\n",100.0*zeros/PAYLOAD_SIZE,EPA.beamlets);
    } else {
      printf(" OK Zeros in payload:    %.f %%\n",100.0*zeros/PAYLOAD_SIZE);
    }

    if(  rate < 0.99 * EXPECTED_PPS(160) 
     || (rate > 1.01 * EXPECTED_PPS(160) &&
         rate < 0.99 * EXPECTED_PPS(200)) ) {
      printf("NOK Packet rate:         %.f pps (200 MHz: ~%.f; 160 MHz: ~%.f)\n",rate,EXPECTED_PPS(200),EXPECTED_PPS(160));
    } else {
      printf(" OK Packet rate:         %.f pps (200 MHz: ~%.f; 160 MHz: ~%.f)\n",rate,EXPECTED_PPS(200),EXPECTED_PPS(160));
    }
  }


  return 0;
}
