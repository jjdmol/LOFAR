/***************************************************************************\

                OUTILS  C LIBRARY

file name......:    outils.c
description....:    outils divers.
authors........:    Carlos Mejia
history........:
                    28/10/99      original file(CM)

\***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "outils.h"

char            display_error_message = ON;

/**************************************************************************/
/* Author :   C. Mejia                                                    */
/* Date   :   28/02/2000                                                  */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :  fonction C qui remplace la macro 'raise_error'. Garde la   */
/*             structure.                                                 */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
raise_error (char *procedure, char *message, int level)
{
  switch (level) {
  case ER_WARNING:
    if (display_error_message)
      error_message ("Warning", procedure, message);
    break;

  case ER_FAIL:
    /* if (display_error_message) */
    error_message ("Fail", procedure, message);
    exit (level);
    break;

  case ER_CRITICAL:
    error_message ("Critical", procedure, message);
    exit (level);
    break;

  case ER_PANIC:
    error_message ("Panic", procedure, message);
    exit (level);
    break;

  default:
    error_message ("Unknown", procedure, message);
    exit (level);
  }
}

/*
 * OurAToF(String) String must contain a number, and nothing else !
 * use 'strtod' :
 *       double strtod(const char *nptr, char **endptr);
 */
double
OurAToF (Token)
     char           *Token;
{
  double          x;
  char           *endPtr;
  char            Message[1024];

  x = strtod (Token, &endPtr);

  sprintf (Message, "Error reading a double float <<%s>>", Token);
  if (*endPtr)
    raise_error ("OurAToF", Message, ER_CRITICAL);

  return x;
}

/*
 * OurAToL(String) String must contain a long integer number, and nothing else !
 * use 'strtol' :
 *       long int strtol(const char *nptr, char **endptr, int base);
 */
long
OurAToL (Token)
     char           *Token;
{
  static int      Base = 0;
  long int        x;
  char           *endPtr;
  char            Message[1024];

  x = strtol (Token, &endPtr, Base);

  sprintf (Message, "Error reading a long integer <<%s>>", Token);
  if (*endPtr)
    raise_error ("OurAToL", Message, ER_CRITICAL);

  return x;
}

/*
 * OurAToI(String) String must contain an integer number, and nothing else !
 * use 'strtol' :
 *       long int strtol(const char *nptr, char **endptr, int base);
 */
int
OurAToI (Token)
     char           *Token;
{
  static int      Base = 0;
  int             x;
  char           *endPtr;
  char            Message[1024];

  x = (int) strtol (Token, &endPtr, Base);

  sprintf (Message, "Error reading an integer <<%s>>", Token);
  if (*endPtr)
    raise_error ("OurAToI", Message, ER_CRITICAL);

  return x;
}

/*
 * OurAToPosI(String) String must contain a positive integer number, and nothing else !
 * use 'strtol' :
 *       long int strtol(const char *nptr, char **endptr, int base);
 */
int
OurAToPosI (Token)
     char           *Token;
{
  static int      Base = 0;
  int             x;
  char           *endPtr;
  char            Message[1024];

  x = (int) strtol (Token, &endPtr, Base);

  sprintf (Message, "Error reading an integer <<%s>>", Token);
  if (*endPtr)
    raise_error ("OurAToPosI", Message, ER_CRITICAL);

  sprintf (Message, "This is not a positive integer <<%s>>", Token);
  if (x < 0)
    raise_error ("OurAToPosI", Message, ER_CRITICAL);

  return x;
}

/* my_strdup(String)
 *
 * redefinie la fonction STRDUP. Est utilisee dans systemes qui n'ont
 * pas cette fonction.
 */
char           *
my_strdup (char *s)
{
  int             len = strlen (s);
  char           *p = (char *) malloc (len + 1);

  strcpy (p, s);
  return p;
}

int
non_interactive_read_arguments (char *fName, char *(*pArg[]))
{
  FILE           *fp;
  char           *pName;
  char            FileName[1024];
  char            InBuffer[1024];
  int             argc = 0;

  if ( (pName = strrchr(fName, DIR_SEPARATOR)) )
    ++ pName;
  else
    pName = fName;

  sprintf (FileName, "%s.arg", pName + 1);

  fp = fopen (FileName, "r");
  
  if (fp) {
    printf("\n      ### ##################################################\n");
    printf("      ###  Programme : %s\n", pName);
    printf("      ###  Lecture non interactive des arguments :\n");
    printf("      ###    Nom du fichier ..... %s\n", FileName);
    while (fscanf (fp, "%s", InBuffer) == 1) {
      ++argc;
      (*pArg)[argc - 1] = strdup (InBuffer);
      printf("      ###    argument %-4d ...... %s\n", argc, (*pArg)[argc - 1]);
    }
    printf("      ### ##################################################\n\n");
  }

  return (argc);
}
