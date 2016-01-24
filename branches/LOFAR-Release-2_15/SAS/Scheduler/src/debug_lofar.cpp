/*
 * debug_lofar.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 27, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/debug_lofar.cpp $
 *
 */

#include "lofar_scheduler.h"
#ifdef DEBUG_SCHEDULER
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#endif

void debugInfo( std::string szTypes, ... ) {
#ifdef DEBUG_SCHEDULER
   va_list vl;
   size_t i;

   va_start( vl, szTypes );

   std::cout << "Info: ";
   for( i = 0; i != szTypes.size(); ++i ) {
      union Printable_t {
         int     i;
         float   f;
         char    c;
         char   *s;
      } Printable;

      switch( szTypes.at(i) ) {   // Type to expect.
         case 'i':
        	Printable.i = va_arg( vl, int );
            std::cout << Printable.i;
         break;

         case 'f':
             Printable.f = va_arg( vl, double );
             std::cout << Printable.f;
         break;

         case 'c':
        	 Printable.c = va_arg( vl, int );
             std::cout << Printable.c;
         break;

         case 's':
             Printable.s = va_arg( vl, char * );
             std::cout << Printable.s;
         break;

         default:
         break;
      }
   }
   std::cout << std::endl;
   va_end( vl );
#endif
}

void debugWarn( std::string szTypes, ... ) {
#ifdef DEBUG_SCHEDULER
	va_list vl;
	size_t i;

	va_start( vl, szTypes );

	std::cout << "Warning: ";
	for( i = 0; i != szTypes.size(); ++i ) {
		union Printable_t {
			int     i;
			float   f;
			char    c;
			char   *s;
		} Printable;

		switch( szTypes.at(i) ) {   // Type to expect.
		case 'i':
			Printable.i = va_arg( vl, int );
			std::cout << Printable.i;
			break;

		case 'f':
			Printable.f = va_arg( vl, double );
			std::cout << Printable.f;
			break;

		case 'c':
			Printable.c = va_arg( vl, int );
			std::cout << Printable.c;
			break;

		case 's':
			Printable.s = va_arg( vl, char * );
			std::cout << Printable.s;
			break;

		default:
			break;
		}
	}
	std::cout << std::endl;
	va_end( vl );
#endif
}

void debugErr( std::string szTypes, ... ) {
#ifdef DEBUG_SCHEDULER
	va_list vl;
	size_t i;

	va_start( vl, szTypes );

	std::cerr << "Error: ";
	for( i = 0; i != szTypes.size(); ++i ) {
		bool stop = false;
		//   for( i = 0; va_arg(vl,int) != -1; ++i ) {
		union Printable_t {
			int     i;
			unsigned int u;
			double   f;
			char    c;
			char   *s;
		} Printable;

		switch( szTypes.at(i) ) {   // Type to expect.
		case 'i':
			Printable.i = va_arg( vl, int );
			if (Printable.i) { std::cerr << Printable.i; }
			else { stop = true; break;}
			break;

		case 'u':
			Printable.u = va_arg( vl, unsigned int );
			if (Printable.u) { std::cerr << Printable.u; }
			else { stop = true; break;}
			break;

		case 'f':
			Printable.f = va_arg( vl, double );
			if (Printable.f) { std::cerr << Printable.f; }
			else { stop = true; break;}
			break;

		case 'c':
			Printable.c = va_arg( vl, int );
			if (Printable.c) { std::cerr << Printable.c; }
			else { stop = true; break;}
			break;

		case 's':
			Printable.s = va_arg( vl, char * );
			if (Printable.s) { std::cerr << Printable.s; }
			else { stop = true; break;}
			break;

		default:
			break;
		}
		if (stop) break;
	}
	std::cerr << std::endl;
	va_end( vl );
#endif
}


