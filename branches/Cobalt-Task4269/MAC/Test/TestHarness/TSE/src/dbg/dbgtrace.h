/*****************************************************************************/
/*                                                                           */
/* File       : DbgTrace.h                                                */
/*                                                                           */
/* Start date : 2000-11-08                                                   */
/*                                                                           */
/* Author     : AU-System Lars Eberson (lars.eberson@ausys.se)               */
/*                                                                           */
/* Version    : 0.0                                                          */
/*                                                                           */
/* Description: Macro definitions for debug outputs in Windows debugger      */
/*                                                                           */
/*****************************************************************************/
#ifndef __DBG_TRACE_H_
#define __DBG_TRACE_H_
/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef WIN32
#include <Windows.h>
#endif 

/*****************************************************************************/
/*                                                                           */
/* External references                                                       */
/*                                                                           */
/*****************************************************************************/
extern char chDbgBuf[8192];
#ifndef WIN32
extern void StartDebugger     ( void );
extern void OutputDebugString (char *);
extern void StopDebugger      ( void );

#endif

/*****************************************************************************/
/*                                                                           */
/* Constants                                                                 */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Macro definitions                                                         */
/*                                                                           */
/*****************************************************************************/

#ifdef DEBUG_AUTOTEST

#define DbgTrace1(a)             {OutputDebugString(a);}
#define DbgTrace2(a,b)           {sprintf(chDbgBuf,a,b); OutputDebugString(chDbgBuf);}
#define DbgTrace3(a,b,c)         {sprintf(chDbgBuf,a,b,c); OutputDebugString(chDbgBuf);}
#define DbgTrace4(a,b,c,d)       {sprintf(chDbgBuf,a,b,c,d); OutputDebugString(chDbgBuf);}
#define DbgTrace5(a,b,c,d,e)     {sprintf(chDbgBuf,a,b,c,d,e); OutputDebugString(chDbgBuf);}
#define DbgTrace6(a,b,c,d,e,f)   {sprintf(chDbgBuf,a,b,c,d,e,f); OutputDebugString(chDbgBuf);}

#else

#define DbgTrace1(a)
#define DbgTrace2(a,b)
#define DbgTrace3(a,b,c)
#define DbgTrace4(a,b,c,d)
#define DbgTrace5(a,b,c,d,e)
#define DbgTrace6(a,b,c,d,e,f)

#endif /* DEBUG_AUTOTEST */

#endif /* __DBG_TRACE_H_ */
