/* $Id$ */

/*-------------------------------------------------------------------------*\
|     Defines for the presence or absence of (system) header files          |
\*-------------------------------------------------------------------------*/

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the <net/ethernet.h> header file. */
#cmakedefine HAVE_NET_ETHERNET_H 1

/* Define to 1 if you have the <netdb.h> header file. */
#cmakedefine HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#cmakedefine HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <sys/mman.h> header file. */
#cmakedefine HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/resource.h> header file. */
#cmakedefine HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/timepps.h> header file. */
#cmakedefine HAVE_SYS_TIMEPPS_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1


/*-------------------------------------------------------------------------*\
|     Defines for the presence or absence of (system) types                 |
\*-------------------------------------------------------------------------*/

/* Define if `long long' is supported */
#cmakedefine HAVE_LONG_LONG 1

/* Define if `uint' is supported */
#cmakedefine HAVE_UINT 1

/* Define if `ulong' is supported */
#cmakedefine HAVE_ULONG 1

/* Define if `ushort' is supported */
#cmakedefine HAVE_USHORT 1

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#cmakedefine WORDS_BIGENDIAN 1


/*-------------------------------------------------------------------------*\
|     Defines for the presence or absence of (system) libraries             |
\*-------------------------------------------------------------------------*/

/* Define if AIPS++ is installed */
#cmakedefine HAVE_AIPSPP

/* Define if libbfd is available */
#cmakedefine HAVE_BFD 1

/* Define if Blitz is installed */
#cmakedefine HAVE_BLITZ 1

/* Define if BOOST is installed */
#cmakedefine HAVE_BOOST 1

/* Define if BOOST component regex is installed */
#cmakedefine HAVE_BOOST_REGEX 1

/* Define if BOOST component date_time is installed */
#cmakedefine HAVE_BOOST_DATE_TIME 1

/* Define if CASACORE is installed with the requested components */
#cmakedefine HAVE_CASACORE 1

/* Define if DAL is installed */
#cmakedefine HAVE_DAL

/* Define if FFTW2 is installed */
#cmakedefine HAVE_FFTW2

/* Define if FFTW3 is installed */
#cmakedefine HAVE_FFTW3

/* Define if libgcrypt is installed */
#cmakedefine HAVE_GCRYPT 1

/* Define if HDF5 is installed */
#cmakedefine HAVE_HDF5

/* Define if LAM is installed */
#cmakedefine HAVE_LAM 1

/* Define if libnuma is installed */
#cmakedefine HAVE_LIBNUMA 1

/* Define if libssh2 is installed */
#cmakedefine HAVE_LIBSSH2 1

/* Define if libxml++ is installed */
#cmakedefine HAVE_LIBXMLXX 1

/* Define if LOG4CPLUS is installed */
#cmakedefine HAVE_LOG4CPLUS 1

/* Define if LOG4CXX is installed */
#cmakedefine HAVE_LOG4CXX 1

/* Define if we have an MPI implementation installed */
#cmakedefine HAVE_MPI 1

/* Define if MPICH is installed */
#cmakedefine HAVE_MPICH 1

/* Define if libopenssl + libcrypto is installed */
#cmakedefine HAVE_OPENSSL 1

/* Define if libpqxx is installed */
#cmakedefine HAVE_PQXX

/* Define if PVSS is installed */
#cmakedefine HAVE_PVSS 1

/* Define if using Rational Purify */
#cmakedefine HAVE_PURIFY 1

/* Define if QPID is installed */
#cmakedefine HAVE_QPID 1

/* Define if readline is installed */
#cmakedefine HAVE_READLINE 1

/* Define if ScaMPI is installed */
#cmakedefine HAVE_SCAMPI 1

/* Defined if shared memory is used */
#cmakedefine HAVE_SHMEM 1

/* Defined if uuid is installed */
#cmakedefine HAVE_UUID 1

/* Define if WCSLIB is installed */
#cmakedefine HAVE_WCSLIB 1


/*-------------------------------------------------------------------------*\
|  Defines for the presence or absence of (system) functions                |
\*-------------------------------------------------------------------------*/

/* Define to __PRETTY_FUNCTION__, __FUNCTION__, or "<unknown>" */
#cmakedefine AUTO_FUNCTION_NAME @AUTO_FUNCTION_NAME@

/* Define to 1 if you have the `backtrace' function. */
#cmakedefine HAVE_BACKTRACE 1

/* Define to 1 if you have a declaration for the `getprotobyname_r' function. */
#cmakedefine HAVE_GETPROTOBYNAME_R 1

/* Define to 1 if you have a declaration for the `strnlen' function. */
#cmakedefine HAVE_STRNLEN 1
