/**************************************************************************/
/* Author :   C. Mejia                                                    */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :   Outils divers utilises par les programmes qui utilisent   */
/* la libreria spoutnik.                                                  */
/* Contient egalement l'ancien block ERROR.H de P. DAIGREMONT (modifie    */
/* par C. AMBROISE) :                                                     */
/* old error.h, type, define and proto for error processing.              */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_OUTILS__
#define __SPOUTNIK_OUTILS__


#define DIR_SEPARATOR '/'

#define Show(f,x) { printf(f,x); fflush(stdout); }

#define ER_WARNING  1
#define ER_FAIL     2
#define ER_CRITICAL 3
#define ER_PANIC    4

#define SetDisplayMessage( onOff ) ( display_spoutnik_message = onOff )
#define ON  1
#define OFF 0

#define error_message(level,procedure,message) \
  fprintf (stderr, "\n%s : %s : %s\n", level, procedure, message)

void            raise_error (char *procedure, char *message, int level);

extern char     display_error_message;

/* OurAToF(String)
 * String must contain a number, and nothing else !
 */
double          OurAToF (
#ifndef NO_PROTOTYPES
			  char *String
#endif
  );

/* OurAToI(String)
 * String must contain an integer number, and nothing else !
 */
int             OurAToI (
#ifndef NO_PROTOTYPES
			  char *String
#endif
  );

/* OurAToL(String)
 * String must contain a long integer number, and nothing else !
 */
long            OurAToL (
#ifndef NO_PROTOTYPES
			  char *String
#endif
  );

/* OurAToPosI(String)
 * String must contain a positive integer number, and nothing else !
 */
int             OurAToPosI (
#ifndef NO_PROTOTYPES
			     char *String
#endif
  );

/* my_strdup(String)
 *
 * redefinie la fonction STRDUP. Est utilisee dans systemes qui n'ont
 * pas cette fonction.
 */
char           *my_strdup (
#ifndef NO_PROTOTYPES
			    char *s
#endif
  );

#ifndef strdup
#define strdup(s) my_strdup(s)
#endif

#endif

int             non_interactive_read_arguments (char *argv0, char *(*pargv[]));
