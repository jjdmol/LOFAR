/*programme destine a tourner sous Unix */

/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<<
   >>>>
   >>>>            File Title
   >>>> copy.o \

   >>>>  Static:
   >>>>             _static_routines()
   >>>>  Public:
   >>>>             public_routines()
   >>>>
   >>>>>>>>>>>>>>>>>>>>>>>>>>>>> <<<<<<<<<<<<<<<<<<<<<<<<<< */

/* #include <values.h> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "outils.h"
#include "spoutnik.h"

/**************************************************************************/
/* Author : Philippe DAIGREMONT                                           */
/* Date   : 04-06-95                                                      */
/* File   : file.c                                                        */
/**************************************************************************/
/* Abstract : bibliotheque de routine de gestion des fichiers             */
/*                                                                        */
/* ces routines permettent de gerer les fichiers utilises sous la forme   */
/* d'un ensemble de fichiers.                                             */
/* Le principe est de constituer un catalogue des fichiers ouverts par    */
/* le programme au debut du programme                                     */
/*                                                                        */
/**************************************************************************/
/* Parameters : neant                                                     */
/**************************************************************************/

/*========================================================================*/
/*                                 NewFile                                */
/*-Parameters-------------------------------------------------------------*/
/*   i/o : f         Descripteur de fichier a creer                       */
/*   in  : fileName  Nom du fichier                                       */
/*   in  : openMode  Mode d'ouverture du fichier                          */
/*                   FS_OUTPUT_MODE / FS_INPUT_MODE /                     */
/*                   FS_APPEND_MODE / FS_OVERWRITE_MODE                   */
/*   in  : header    output file -> entete du fichier                     */
/*                   input file  -> chaine vide                           */
/*-Return-----------------------------------------------------------------*/
/*   neant                                                                */
/*-Examples---------------------------------------------------------------*/
/*   file_t outFile;                                                      */
/*   NewFile( &outFile, "myOutput.txt", FS_OUTPUT_MODE );                 */
/*-Abstract---------------------------------------------------------------*/
/*   Cette fonction a pour objet d'allouer la place du descripteur de     */
/*   fichier, de l'initialiser, et d'ouvrir le fichier correspondant.     */
/*-Dependencies-----------------------------------------------------------*/
/* call     : GetOpenMode                                                 */
/* call by  : AddFile                                                     */
/*========================================================================*/

void
NewFile (file_t * f, char *fileName, int openMode, char *header)
{
  /*  char * str = NULL; */
  size_t          length;

  switch (openMode) {
  case FS_TEST_INPUT_MODE:
  case FS_INPUT_MODE:
    /*
     * INPUT_MODE: ouverture en lecture,
     * test d'existence du fichier.
     * Abort si fichier inexistent.
     */
    {
      char            BffMsg[FS_RECORD_SIZE];
      FILE           *fptemp = fopen (fileName, "r");

      if (fptemp == (FILE *) NULL) {
	sprintf (BffMsg, "attention, input file <<%s>> do not exist", fileName);
	raise_error ("NewFile", BffMsg, ER_CRITICAL);
      }
      fclose (fptemp);
    }
    break;

  case FS_TEST_OUTPUT_MODE:
  case FS_OUTPUT_MODE:
    /*
     * OUTPUT_MODE: ouverture en ecriture
     * test de non existence du fichier.
     * Empeche l'ecrasement d'un fichier --> abort.
     */
    {
      char            BffMsg[FS_RECORD_SIZE];
      FILE           *fptemp = fopen (fileName, "r");

      if (fptemp) {
	sprintf (BffMsg,
		 "attention, output file <<%s>> do exist. Errase it or give another name",
		 fileName);
	fclose (fptemp);
	raise_error ("NewFile", BffMsg, ER_CRITICAL);
      }
    }
    break;

  case FS_OVERWRITE_MODE:
    /*
     * OVERWRITE_MODE: ouverture en ecriture
     * pas de test de non existence du fichier.
     * PERMET L'ECRASEMENT D'UN FICHIER.
     */
#ifdef DEBUG
    {
      char            BffMsg[FS_RECORD_SIZE];
      FILE           *fptemp = fopen (fileName, "r");

      if (fptemp) {
	sprintf (BffMsg,
		 "warning, output file <<%s>> overwrote",
		 fileName);
	fclose (fptemp);
	raise_error ("NewFile", BffMsg, ER_WARNING);
      }
    }
#endif
    break;
  case FS_APPEND_MODE:
    /*
     * APPEND_MODE: ouverture en mode append
     * test d'existence et non existence du fichier.
     * Annonce la creation ou bien la reecriture.
     * L'execution du programme continue.
     */
    {
      char            BffMsg[FS_RECORD_SIZE];
      FILE           *fptemp = fopen (fileName, "r");

      if (fptemp) {
	sprintf (BffMsg,
		 "attention, append file <<%s>> do exist. Stream is positioned at the end of the file",
		 fileName);
	fclose (fptemp);
      } else
	sprintf (BffMsg, "attention, append file <<%s>> do not exist. File will be created",
		 fileName);
      raise_error ("NewFile", BffMsg, ER_WARNING);
    }
  }

  if ((openMode != FS_TEST_INPUT_MODE) && (openMode != FS_TEST_OUTPUT_MODE)) {
    *f = malloc (sizeof (fileDesc_t));
    if (*f == NULL)
      raise_error ("NewFile", "malloc failed (1)", ER_FAIL);

    length = strlen (fileName) + 1;

    (*f)->fileName = (char *) malloc (length * sizeof (char));
    if ((*f)->fileName == NULL)
      raise_error ("NewFile", "malloc failed (2)", ER_FAIL);

    strncpy ((*f)->fileName, fileName, length);
    (*f)->openMode = openMode;
    (*f)->nextFile = NULL;
    (*f)->filePointer = fopen ((*f)->fileName, GetOpenMode ((*f)));
    if ((*f)->filePointer == NULL)
      raise_error ("NewFile", "fopen failed", ER_FAIL);

    if ((openMode == FS_OUTPUT_MODE) || (openMode == FS_OVERWRITE_MODE))
      SetHeader ((*f), header);
    else {
      (*f)->header = (char *) malloc (FS_HEADER_SIZE * sizeof (char));
      if ((*f)->header == NULL)
	raise_error ("NewFile", "malloc failed (3)", ER_FAIL);

      fgets ((*f)->header, FS_HEADER_SIZE, (*f)->filePointer);
      if ((*f)->header == NULL)
	raise_error ("NewFile", "no header", ER_WARNING);
    }
  }

  return;
}

/*========================================================================*/
/*                                FreeFile                                */
/*-Parameters-------------------------------------------------------------*/
/*   i/o : f         Descripteur de fichier a liberer                     */
/*-Return-----------------------------------------------------------------*/
/*   neant                                                                */
/*-Examples---------------------------------------------------------------*/
/*   FreeFile( &outFile );                                                */
/*-Abstract---------------------------------------------------------------*/
/*   Cette fonction a pour objet de fermer le fichier correspondant au    */
/*   descripteur, puis de liberer la memoire utilisee par le descripteur  */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  : FreeFileSet                                                 */
/*========================================================================*/

void
FreeFile (file_t * f)
{
  if ((*f)->filePointer != NULL)
    fclose ((*f)->filePointer);
  if ((*f)->header != NULL)
    free ((*f)->header);
  free ((*f)->fileName);
  free (*f);
  *f = NULL;

  return;
}

/*========================================================================*/
/*                              ResetFile                                 */
/*-Parameters-------------------------------------------------------------*/
/*   i/o : f         Descripteur de fichier a reouvrir                    */
/*-Return-----------------------------------------------------------------*/
/*   neant                                                                */
/*-Examples---------------------------------------------------------------*/
/*   ResetFile( outFile );                                                */
/*-Abstract---------------------------------------------------------------*/
/*   Cette fonction a pour objet de fermer le fichier correspondant au    */
/*   descripteur, puis de le reouvrir                                     */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
ResetFile (file_t * f)
{
  if ((*f)->filePointer != NULL) {
    fflush((*f)->filePointer);
    rewind ((*f)->filePointer);
  } else {
    (*f)->filePointer = fopen ((*f)->fileName, GetOpenMode (*f));

    if ((*f)->filePointer == NULL)
      raise_error ("ResetFile", "fopen failed", ER_PANIC);
  }

  if ((*f)->openMode == FS_INPUT_MODE) {
    free ((*f)->header);
    (*f)->header = (char *) malloc (FS_HEADER_SIZE * sizeof (char));

    if ((*f)->header == NULL)
      raise_error ("ResetFile", "malloc failed", ER_FAIL);

    fgets ((*f)->header, FS_HEADER_SIZE, (*f)->filePointer);
    if ((*f)->header == NULL)
      raise_error ("ResetFile", "no header", ER_WARNING);
  }
  return;
}

/*========================================================================*/
/*                              NewFileSet                                */
/*-Parameters-------------------------------------------------------------*/
/*   i/o : fileSet   Descripteur de catalogue de fichier a creer          */
/*-Return-----------------------------------------------------------------*/
/*   neant                                                                */
/*-Examples---------------------------------------------------------------*/
/*   fileSet_t fileSet;                                                   */
/*   NewFileSet( &fileSet );                                              */
/*-Abstract---------------------------------------------------------------*/
/*   Cette fonction a pour objet d'allouer la memoire necessaire au       */
/*   descripteur de catalogue de fichier et de l'initialiser.             */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
NewFileSet (fileSet_t * fileSet)
{
  *fileSet = malloc (sizeof (fileSetDesc_t));
  if (*fileSet == NULL)
    raise_error ("NewFileSet", "malloc failed", ER_PANIC);
  (*fileSet)->firstFile = NULL;
  (*fileSet)->curFile = NULL;

  return;
}

/*========================================================================*/
/*                              FreeFileSet                               */
/*-Parameters-------------------------------------------------------------*/
/*   i/o : fileSet   Descripteur de catalogue de fichier a liberer        */
/*-Return-----------------------------------------------------------------*/
/*   neant                                                                */
/*-Examples---------------------------------------------------------------*/
/*   FreeFileSet( &fileSet );                                             */
/*-Abstract---------------------------------------------------------------*/
/*   Cette fonction a pour objet de liberer tout les fichiers du catalogue */
/*   puis de liberer la memoire occupee par le catalogue.                 */
/*-Dependencies-----------------------------------------------------------*/
/* call     : FileSetIsEmpty                                              */
/*            FreeFile                                                    */
/* call by  :                                                             */
/*========================================================================*/

void
FreeFileSet (fileSet_t * fileSet)
{
  while (!FileSetIsEmpty ((*fileSet))) {
    (*fileSet)->curFile = (*fileSet)->firstFile;
    (*fileSet)->firstFile = (*fileSet)->curFile->nextFile;
    FreeFile (&((*fileSet)->curFile));
  }
  free (*fileSet);
  *fileSet = NULL;

  return;
}

/*========================================================================*/
/*                                AddFile                                 */
/*-Parameters-------------------------------------------------------------*/
/*   in  : fileSet   Descripteur de catalogue de fichier                  */
/*   in  : fileName  Nom du fichier a ajouter au catalogue                */
/*   in  : openMode  Mode d'ouverture du fichier                          */
/*                   FS_OUTPUT_MODE / FS_INPUT_MODE                       */
/*   in  : header    output file -> entete du fichier                     */
/*                   input file  -> chaine vide                           */
/*-Return-----------------------------------------------------------------*/
/*   neant                                                                */
/*-Examples---------------------------------------------------------------*/
/*   AddFile( fileSet, "myOutput.txt", FS_OUTPUT_MODE );                  */
/*-Abstract---------------------------------------------------------------*/
/*   Cette fonction a pour objet d'ajouter un fichier au catalogue et     */
/*   d'ouvrir ce fichier dans le mode desire.                             */
/*-Dependencies-----------------------------------------------------------*/
/* call     : NewFile                                                     */
/* call by  :                                                             */
/*========================================================================*/

void
AddFile (fileSet_t fileSet, char *fileName, int openMode, char *header)
{
  fileSet->curFile = fileSet->firstFile;
  NewFile (&(fileSet->firstFile), fileName, openMode, header);
  if ((fileSet->firstFile != fileSet->curFile)
      && (fileSet->firstFile != NULL)) {
    fileSet->firstFile->nextFile = fileSet->curFile;
  } else {
    fileSet->firstFile = fileSet->curFile;
    raise_error ("AddFile", "file not added", ER_FAIL);
  }

  return;
}

/*========================================================================*/
/*                             GetFileByName                              */
/*-Parameters-------------------------------------------------------------*/
/*   in  : fileSet   Descripteur de catalogue de fichier                  */
/*   in  : fileName  Nom du fichier dont on veut le pointeur de fichier   */
/*-Return-----------------------------------------------------------------*/
/*   descripteur de fichier correspondant au nom du fichier en entree     */
/*   ou NULL s'il n'existe pas de fichier a ce nom dans le catalogue      */
/*-Examples---------------------------------------------------------------*/
/*   FILE *myFile;                                                        */
/*   myFile = GetFileByName( fileSet, "myOutput.txt" );                   */
/*-Abstract---------------------------------------------------------------*/
/*   Cette fonction a pour objet de retrouver dans le catalogue le        */
/*   descripteur de fichier correspondant au nom de fichier.              */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

file_t
GetFileByName (fileSet_t fileSet, char *fileName)
{
  file_t          foundFile;

  foundFile = NULL;
  fileSet->curFile = fileSet->firstFile;
  while ((fileSet->curFile != NULL) && (foundFile == NULL)) {
    if (strcmp (fileSet->curFile->fileName, fileName) == 0)
      foundFile = fileSet->curFile;
    fileSet->curFile = fileSet->curFile->nextFile;
  }

  if (foundFile == NULL) {
    raise_error ("GetFileByName", "file not found", ER_WARNING);
  }
  return (foundFile);
}

/*========================================================================*/
/*                             GetParameter                                */
/*-Parameters-------------------------------------------------------------*/
/*   in  : char *tag :Researched substring which allows paramter location */
/*   in  : char *str :String supposed to contain  tag and parameter       */
/*   out : char *parameter : string containing the researched  parameter  */
/*-Return-----------------------------------------------------------------*/
/*    string containing the researched  parameter                         */
/*-Examples---------------------------------------------------------------*/
/*   char *parameter=NULL;                                                */
/*   parameter = GetParameter(parameter,"mapDim=",header);                */
/*-Abstract---------------------------------------------------------------*/
/*  This function searches for a string containing a parameter. It is used */
/*   to extract information from file headers                             */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

char           *
GetParameter (char *parameter, char *tag, char *str)
{
  int             argLength;
  int             parameterLength;
  int             length;
  char           *tagPos = NULL;
  char           *arg = NULL;
  char           *format = NULL;

  FreeParameter (parameter);
  arg = (char *) calloc (FS_RECORD_SIZE, sizeof (char));
  format = (char *) calloc (FS_RECORD_SIZE, sizeof (char));
  if ((arg == NULL) || (format == NULL))
    raise_error ("GetParameter", "malloc failed (1)", ER_FAIL);

  tagPos = strstr (str, tag);
  if (tagPos == NULL) {
    raise_error ("GetParameter", "tag not found", ER_WARNING);
    free (arg);
    free (format);
    return (parameter);
  }
  length = strcspn (tagPos, " ");
  strncpy (arg, tagPos, (unsigned) length);
  if (arg == NULL) {
    raise_error ("GetParameter", "argument not found", ER_WARNING);
    free (arg);
    free (format);
    return (parameter);
  }
  parameterLength = argLength - (int) strlen (tag);
/*  parameter = (char *)malloc( (parameterLength+1)*sizeof(char) );  */

  parameter = (char *) calloc (FS_RECORD_SIZE, sizeof (char));
  if (parameter == NULL)
    raise_error ("GetParameter", "malloc failed (2)", ER_FAIL);

  length = strlen (tag);
  strncpy (format, tag, (unsigned) length);
  strncpy ((format + length), "%s", 3);
  sscanf (arg, format, parameter);
  if (parameter == NULL)
    raise_error ("GetParameter", "parameter not found", ER_CRITICAL);

  free (arg);
  free (format);
  return (parameter);
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
NewVector (vector_t * vector, long dim)
{
  *vector = (vector_t) malloc (sizeof (vectorDesc_t));
  if (!(*vector))
    raise_error ("NewVector", "malloc failed (1)", ER_FAIL);

  (*vector)->dim = dim;
  (*vector)->values = (double *) calloc ((size_t) dim, sizeof (double));

  if (!(*vector)->values)
    raise_error ("NewVector", "malloc failed (2)", ER_FAIL);

/* pointeur index cree pour les neurones uniquement
   a NULL par defaut */
  (*vector)->index = NULL;

  return;
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
FreeVector (vector_t * vector)
{
  if (*vector) {
    free ((*vector)->values);
    free ((*vector));
    *vector = NULL;
  }
}

/**************************************************************************/
/* Author :   C. Ambroise                                                 */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
NewMatrix (matrix_t * matrix, long dimi, long dimj)
{
  *matrix = (matrix_t) malloc (sizeof (matrixDesc_t));
  if (*matrix == NULL)
    raise_error ("NewMatrix", "malloc failed (1)", ER_FAIL);

  (*matrix)->dimi = dimi;
  (*matrix)->dimj = dimj;
  (*matrix)->values = (double *) calloc ((size_t) dimi * dimj, sizeof (double));

  if ((*matrix)->values == NULL)
    raise_error ("NewMatrix", "malloc failed (2)", ER_FAIL);

  return;
}

/**************************************************************************/
/* Author :   C. Ambroise                                                 */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
FreeMatrix (matrix_t * matrix)
{
  if (*matrix) {
    free ((*matrix)->values);
    free ((*matrix));
    *matrix = NULL;
  }
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
CopyVector (vector_t from, vector_t to)
{
  long            i;

  if (from->dim != to->dim)
    raise_error ("CopyVector", "incompatible vectors", ER_FAIL);

  for (i = 0; i < to->dim; i++)
    to->values[i] = from->values[i];

  return;
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

int
StrToVector (char *str, vector_t vector)
{
  long            i;
  char           *field;

  field = str + strspn (str, " {");
  for (i = 0; i < vector->dim; i++) {
    if (sscanf (field, "%lf", &(vector->values[i])) == 0) {
      raise_error ("StrToVector", "incompatible string and vector size", ER_WARNING);
      return i+1;
    }
    if (i < (vector->dim)) {
      field += strspn (field, "+-0123456789.eE");
      field += strspn (field, ", ");
    }
  }

  /* test de fin de vector-string */
  field += strspn (field, " ");
  if (field[0] != '}') {
    char Bff[BFFSIZE];
    sprintf(Bff,"string greater than expected, has more than %ld elements", vector->dim);
    raise_error ("StrToVector", Bff, ER_WARNING);
  }

  return (0);
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

char           *
VectorToStr (vector_t vector, char *str)
{
  long            i;

  sprintf (str, "{");
  for (i = 0; i < vector->dim; i++) {
    sprintf (str, "%s%e", str, vector->values[i]);
    if (i < (vector->dim) - 1)
      sprintf (str, "%s,", str);
  }
  sprintf (str, "%s}", str);

  return (str);
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

int
VectorsAreEquals (vector_t left, vector_t right)
{
  int             bool = 0;
  long            i;

  for (i = 0, bool = 1; i < left->dim; i++)
    bool = ((left->values[i] == right->values[i]) && bool);

  return (bool);
}

/* vector_t SubstractVectors (vector_t res, vector_t left, vector_t right)
{
  long            i;

  for (i = 0; i < res->dim; i++)
    res->values[i] = left->values[i] - right->values[i];

  return (res);
} */

void
DoSubstractVectors (register scalar_t * pDest, register scalar_t * pLeft,
		    register scalar_t * pRight, register long Size)
{
  register long   i;

  for (i = 0; i < Size; i++)
    *pDest++ = *pLeft++ - *pRight++;
}
/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

vector_t
AddVectors (vector_t res, vector_t left, vector_t right)
{
  long            i;

  for (i = 0; i < res->dim; i++)
    res->values[i] = left->values[i] + right->values[i];

  return (res);
}

/* scalar_t OLDEuclidNorm (vector_t vector)
{
  long            i;
  double          sum;

  for (i = 0, sum = 0.0; i < vector->dim; i++)
    sum += Square (vector->values[i]);

  return (sqrt (sum));
} */

scalar_t
DoEuclidNorm (register scalar_t * Vect, register long Size)
{
  register long   i;
  register double sum = 0;

  for (i = 0; i < Size; i++, Vect++)
    sum += Square (*Vect);

  return (sqrt (sum));
}

scalar_t
EuclidNorm2 (vector_t vector)
{
  long            i;
  double          sum;

  for (i = 0, sum = 0.0; i < vector->dim; i++)
    sum += Square (vector->values[i]);

  return (sum);
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

scalar_t
AbsNorm (vector_t vector)
{
  long            i;
  double          sum;

  for (i = 0, sum = 0.0; i < vector->dim; i++)
    sum += fabs (vector->values[i]);

  return (sum);
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

scalar_t
AbsNorm2D (vector_t vector)
{
  return (fabs (vector->values[0]) + fabs (vector->values[1]));
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

scalar_t
QuadraticNorm (vector_t vector)
{
  long            i;
  double          sum;

  for (i = 0, sum = 0.0; i < vector->dim; i++)
    sum += Square (vector->values[i]);

  return (sum);
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

scalar_t
QuadraticDistance (vector_t left, vector_t right)
{
  long            i;
  double          sum;
  double          delta;

  for (i = 0, sum = 0.0; i < left->dim; i++) {
    delta = left->values[i] - right->values[i];
    sum += Square (delta);
  }

  return (sum);
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
NewSet (set_t * set, long size, long dim)
{
  *set = (set_t) malloc (sizeof (setDesc_t));

  if (*set == NULL)
    raise_error ("NewSet", "malloc failed (1)", ER_FAIL);

  (*set)->size = size;
  (*set)->dim = dim;
  (*set)->index = 0;
  (*set)->moments = NULL;
  NewVector (&((*set)->stat), SET_STATSIZE);
  (*set)->firstElement = (element_t) malloc (sizeof (elementDesc_t) * size);

  if ((*set)->firstElement == NULL)
    raise_error ("NewSet", "malloc failed (2)", ER_FAIL);

  (*set)->curElement = (*set)->firstElement;
  ForAllElements ((*set), (*set)->curElement)
    NewElement ((*set)->curElement, dim);
}
/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
FreeSet (set_t * set)
{
  ForAllElements ((*set), (*set)->curElement)
    FreeElement ((*set)->curElement);
  if ((*set)->moments != NULL)
    FreeVector (&((*set)->moments));
  FreeVector (&((*set)->stat));
  free ((*set)->firstElement);
  free ((*set));
  *set = NULL;
}
/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
LoadSet (set_t * set, file_t setFile)
{
  char            msg[FS_RECORD_SIZE];
  char           *str = NULL;

  /*char *begin="Loading "; */
  char            record[FS_RECORD_SIZE];
  char           *data;
  char           *field;
  long            size;
  long            dim;
  int             i;
  long            no = 1;

  if (setFile != NULL) {
    str = GetParameter (str, "setSize=", GetHeader (setFile));
    if (str == NULL)
      raise_error ("LoadSet", "No size specified : not a set file", ER_FAIL);
    StrToLong (str, size);
    FreeParameter (str);
    str = GetParameter (str, "setDim=", GetHeader (setFile));
    if (str == NULL)
      raise_error ("LoadSet", "No dim specified : not a set file", ER_FAIL);
    StrToLong (str, dim);
    FreeParameter (str);

    Show ("\nLoading %s", setFile->fileName);

    if (!SetIsFree (*set)) {
      raise_error ("LoadSet", "set overloaded", ER_WARNING);
      FreeSet (set);
    }
    NewSet (set, size, dim);

    str = GetParameter (str, "setStat=", GetHeader (setFile));
    if (str != NULL) {
      int n_elem = StrToVector(str, (*set)->stat);
      int v_size = (*set)->stat->dim;
      if (n_elem) {
	char Bff[BFFSIZE];
	register int i;
	sprintf(Bff,"\n\t *** Incompatible setStat string than expected vector size (%d < %d). In file :\n\t *** %s\n\t *** Execution continues. Missing data are filled with ZERO",
		n_elem - 1, v_size, setFile->fileName);
	raise_error ("LoadSet", Bff, ER_WARNING);
	for (i = n_elem - 1; i < v_size; ++i)
	  (*set)->stat->values[i] = 0.0;
      }
    }
    FreeParameter (str);

    ForAllElements ((*set), (*set)->curElement) {
      do {
	fgets (record, FS_RECORD_SIZE, GetFilePointer (setFile));
	no++;
	data = strtok (record, "#");
      } while ((strlen (data) == strspn (data, " \t\n")) || record[0] == '#');

      if (strspn (data, " \t+-.0123456789eE\n") != strlen (data)) {
	sprintf (msg, "Bad data on line %lu (element %lu)", no, (*set)->index);
	raise_error ("LoadSet", msg, ER_CRITICAL);
      }
      field = data;
      for (i = 0; i < dim; i++) {
	field += strspn (field, FS_WHITE_SPACES);
	sscanf (field, "%lf", &((*((*set)->curElement))->values[i]));
	field += strspn (field, "+-.0123456789");
	/* if (i < dim - 1)
	   field += strspn (field, FS_END_OF_FIELD); */
      }
      /*    if ((*set)->index%(int)((*set)->size/10)==0) */
      /*        Show("%s","."); */
    }
  }
  Show (" Done. (%lu lines read/", no);
  Show ("%lu elements loaded)\n", (*set)->index);

  return;
}
/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
SaveSet (set_t set, file_t setFile)
{
  char            str[FS_RECORD_SIZE];
  char            arg[FS_RECORD_SIZE] = "# setDim=";
  char            arg1[FS_RECORD_SIZE] = "setSize=";
  char            arg2[FS_RECORD_SIZE] = "setStat=";
  int             i;

  if (!SetIsFree (set) && (setFile != NULL)) {
    LongToStr (set->dim, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg1);
    LongToStr (set->size, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg2);
    VectorToStr (set->stat, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, GetHeader (setFile));
    SetHeader (setFile, arg);
    fprintf (GetFilePointer (setFile), "%s\n", GetHeader (setFile));

    ForAllElements (set, set->curElement) {
      for (i = 0; i < set->dim; i++) {
	fprintf (GetFilePointer (setFile), "%f", (*(set->curElement))->values[i]);
	if (i < set->dim - 1)
	  fprintf (GetFilePointer (setFile), FS_END_OF_FIELD);
      }
      fprintf (GetFilePointer (setFile), FS_END_OF_RECORD);
    }
  }
  return;
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
NewIndex (mapindex_t * index, long dim)
{
  *index = (mapindex_t) malloc (sizeof (indexDesc_t));
  if (*index == NULL)
    raise_error ("NewIndex", "malloc failed (1)", ER_FAIL);

  (*index)->dim = dim;
  (*index)->values = (long *) calloc ((size_t) dim, sizeof (long));

  if ((*index)->values == NULL)
    raise_error ("NewIndex", "malloc failed (2)", ER_FAIL);

  return;
}
/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
FreeIndex (mapindex_t * index)
{
  free ((*index)->values);
  free ((*index));
  *index = NULL;
}

/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
CopyIndex (mapindex_t from, mapindex_t to)
{
  long            i;

  if (from->dim != to->dim)
    raise_error ("CopyIndex", "incompatible indexes", ER_FAIL);

  for (i = 0; i < to->dim; i++)
    to->values[i] = from->values[i];

  return;
}
/**************************************************************************/
/* Author :   C. MEJIA                                                    */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract : applatit une structure multidimensionnelle, cree une struct */
/* monodimensionnelle.                                                    */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

void
FlatIndex (mapindex_t * dest, mapindex_t src)
{
  NewIndex( dest, 1);

  (*dest)->values[0] = IndexRange (src);

  return;
}
/**************************************************************************/
/* Author :   P. DAIGREMONT                                               */
/* Date   :                                                               */
/* File   :                                                                */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

long
Offset (mapindex_t index, mapindex_t size)
{
  long            i;
  long            range = 0;
  long            offset = 0;

  for (i = (index->dim) - 2, offset = index->values[i + 1], range = 1; i >= 0; i--) {
    range *= size->values[i + 1];
    offset += index->values[i] * range;
  }

  return (offset);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

mapindex_t
OffsetToIndex (long offset, mapindex_t index, mapindex_t size)
{
  long            i;

  for (i = (index->dim) - 1; i >= 0; i--) {
    index->values[i] = offset % size->values[i];
    offset /= size->values[i];
  }

  return (index);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

mapindex_t
OffsetToIndex2d (long offset, mapindex_t index, mapindex_t size)
{
  index->values[1] = offset % size->values[1];
  index->values[0] = offset / size->values[1];

  return (index);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

long
IndexRange (mapindex_t index)
{
  long            i;
  long            range;

  for (i = 0, range = 1; i < index->dim; i++)
    range *= index->values[i];

  return (range);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

mapindex_t
StrToIndex (char *str, mapindex_t index)
{
  long            i;
  char           *field;
  long            val;

  field = str + strspn (str, " {");
  for (i = 0; i < index->dim; i++) {
    sscanf (field, "%ld", &val);
    index->values[i] = val;
    if (i < (index->dim) - 1) {
      field += strspn (field, "+-0123456789");
      field += strspn (field, ", ");
    }
  }

  return (index);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

char           *
IndexToStr (mapindex_t index, char *str)
{
  long            i;

  sprintf (str, "{");
  for (i = 0; i < index->dim; i++) {
    sprintf (str, "%s%ld", str, index->values[i]);
    if (i < (index->dim) - 1)
      sprintf (str, "%s,", str);
  }
  sprintf (str, "%s}", str);

  return (str);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

int
IndexAreEquals (mapindex_t left, mapindex_t right)
{
  int             bool = 0;
  long            i;

  for (i = 0, bool = 1; i < left->dim; i++)
    bool = ((left->values[i] == right->values[i]) && bool);

  return (bool);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

mapindex_t
SubstractIndex (mapindex_t res, mapindex_t left, mapindex_t right)
{
  long            i;

  for (i = 0; i < res->dim; i++)
    res->values[i] = left->values[i] - right->values[i];

  return (res);
}

/*========================================================================*/
/*                     AddIndex                                           */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*         mapindex_t : an index, which is the sum of the two input indexes  */
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*   Add two index                                                        */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

mapindex_t
AddIndex (mapindex_t res, mapindex_t left, mapindex_t right)
{
  long            i;

  for (i = 0; i < res->dim; i++)
    res->values[i] = left->values[i] + right->values[i];

  return (res);
}

/*========================================================================*/
/*                    IndexDistance                                       */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*   Compute the city block distance between two index                    */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

double
IndexDistance (mapindex_t left, mapindex_t right)
{
  long            i;
  double          sum;

  for (i = 0, sum = 0.0; i < left->dim; i++)
    sum += fabs ((double) (left->values[i]) - (double) (right->values[i]));

  return (sum);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

double
Index2dDistance (mapindex_t left, mapindex_t right)
{
  return (fabs ((double) (left->values[0]) - (double) (right->values[0])) +
	  fabs ((double) (left->values[1]) - (double) (right->values[1])));
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
NewMap (map_t * map, mapindex_t size, vector_t min, vector_t max, long pointDim)
{
  mapindex_t      pointeur;

  printf ("\nNewMap\n");

  *map = (map_t) malloc (sizeof (mapDesc_t));
  if (*map == NULL)
    raise_error ("NewMap", "malloc failed (1)", ER_FAIL);

  NewIndex (&((*map)->size), size->dim);
  CopyIndex (size, (*map)->size);
  NewIndex (&((*map)->index), size->dim);
  (*map)->range = IndexRange (size);
  (*map)->offset = 0;
  NewVector (&((*map)->min), min->dim);
  CopyVector (min, (*map)->min);
  NewVector (&((*map)->max), max->dim);
  CopyVector (max, (*map)->max);

  (*map)->firstPoint = (point_t) malloc (sizeof (pointDesc_t) * (*map)->range);
  if ((*map)->firstPoint == NULL)
    raise_error ("NewMap", "malloc failed (2)", ER_FAIL);

  (*map)->curPoint = (*map)->firstPoint;
  ForAllPoints ((*map), (*map)->curPoint) {
    NewVector ((*map)->curPoint, pointDim);
    NewIndex (&pointeur, (*map)->size->dim);
    OffsetToIndex ((*map)->offset, pointeur, (*map)->size);
    (*(*map)->curPoint)->index = pointeur;
  }

}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
FreeMap (map_t * map)
{
  ForAllPoints ((*map), (*map)->curPoint)
    FreeVector ((*map)->curPoint);
  free ((*map)->firstPoint);
  FreeVector (&((*map)->max));
  FreeVector (&((*map)->min));
  FreeIndex (&((*map)->size));
  FreeIndex (&((*map)->index));
  free ((*map));
  *map = NULL;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
LoadMap (map_t * map, file_t mapFile)
{
  mapindex_t      size;
  vector_t        min;
  vector_t        max;
  long            i;
  long            dim;
  char           *str = NULL;
  char            record[FS_RECORD_SIZE];
  char           *field;

  char           *p;
  int             indexed_file = OFF;

  int n_elem;

  /*char *begin="Loading "; */

  if (mapFile != NULL) {
    p = GetHeader (mapFile);

    if ( *p == '@' )
      indexed_file = ON;

    str = GetParameter (str, "mapDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMap", "file is not a map file", ER_FAIL);
    StrToLong (str, dim);
    NewIndex (&size, dim);
    FreeParameter (str);
    str = GetParameter (str, "mapSize=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMap", "file is not a map file", ER_FAIL);
    StrToIndex (str, size);
    FreeParameter (str);
    str = GetParameter (str, "pointDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMap", "file is not a map file", ER_FAIL);
    StrToLong (str, dim);
    NewVector (&min, dim);
    NewVector (&max, dim);
    FreeParameter (str);
    str = GetParameter (str, "mapMin=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMap", "file is not a map file", ER_FAIL);

    n_elem = StrToVector(str, min);
    if (n_elem) {
      int v_size = min->dim;
      char Bff[BFFSIZE];
      sprintf(Bff,"incompatible MIN_STRING string and expected vector size: (%d < %d). In file :\n\t *** %s\n\t     Size must be %d ***\n",
	      n_elem - 1, v_size, mapFile->fileName, v_size);
      raise_error ("LoadMap", Bff, ER_CRITICAL);
    }

    FreeParameter (str);
    str = GetParameter (str, "mapMax=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMap", "file is not a map file", ER_FAIL);

    n_elem = StrToVector(str, max);
    if (n_elem) {
      int v_size = max->dim;
      char Bff[BFFSIZE];
      sprintf(Bff,"incompatible MAX_STRING string and expected vector size: (%d < %d). In file :\n\t *** %s\n",
	      n_elem - 1, v_size, mapFile->fileName);
      raise_error ("LoadMap", Bff, ER_CRITICAL);
    }

    FreeParameter (str);

    Show ("\nLoading %s\n", mapFile->fileName);

/*  if ( MapIsFree(*map) )
   NewMap( map, size, min, max, dim );
   else
   if ( !IndexAreEquals( (*map)->size, size ) )
   raise_error( "LoadMap", "incompatible maps", ER_FAIL );  */

    NewMap (map, size, min, max, dim);

    ForAllPoints ((*map), (*map)->curPoint) {
      fgets (record, FS_RECORD_SIZE, GetFilePointer (mapFile));
      field = record;

      if ( indexed_file ) {
	int  n, indj;

	for (i = 0; i < dim; i++)
	  (*((*map)->curPoint))->values[i] = 0.0;

	field += strspn (field, FS_WHITE_SPACES);
	sscanf (field, "%d", &n);
	field += strspn (field, "0123456789");

	for (i = 0; i < n; i++) {
	  field += strspn (field, FS_WHITE_SPACES);
	  sscanf (field, "%d", &indj);
	  field += strspn (field, "0123456789");
	  field += strspn (field, FS_WHITE_SPACES);
	  sscanf (field, "%lf", &((*((*map)->curPoint))->values[indj - 1]));
	  field += strspn (field, "+-.0123456789eE");
	}
      } else {
	for (i = 0; i < dim; i++) {
	  field += strspn (field, FS_WHITE_SPACES);
	  sscanf (field, "%lf", &((*((*map)->curPoint))->values[i]));
	  field += strspn (field, "+-.0123456789");
	  /*	if (i < dim - 1) {
		field += strspn (field, "+-.0123456789");
		field += strspn (field, FS_END_OF_FIELD);
		} */
	}
	/*if ((*map)->offset%(int)((*map)->range/10)==0)
	  Show("%s","."); */
      }
    }
  }
  Show ("%s\n", " Done.");

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
LoadMapIndex (map_t * map, file_t mapFile, long **index)
{
  long           *majIndex = *index;
  mapindex_t      size;
  vector_t        min;
  vector_t        max;
  long            i, k;
  long            dim;
  char           *str = NULL;
  char            record[FS_RECORD_SIZE];
  char           *field;
  int             n_elem;

  /*char *begin="Loading "; */

  if (mapFile != NULL) {
    str = GetParameter (str, "mapDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapIndex", "file1 is not a map file", ER_FAIL);
    StrToLong (str, dim);
    NewIndex (&size, dim);
    FreeParameter (str);
    str = GetParameter (str, "mapSize=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapIndex", "file2 is not a map file", ER_FAIL);
    StrToIndex (str, size);
    FreeParameter (str);
    str = GetParameter (str, "pointDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapIndex", "file3 is not a map file", ER_FAIL);
    StrToLong (str, dim);
    NewVector (&min, dim);
    NewVector (&max, dim);
    FreeParameter (str);
    str = GetParameter (str, "mapMin=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapIndex", "file4 is not a map file", ER_FAIL);

    n_elem = StrToVector(str, min);
    if (n_elem) {
      int v_size = min->dim;
      char Bff[BFFSIZE];
      sprintf(Bff,"incompatible MIN_STRING string and expected vector size: (%d < %d). In file :\n\t *** %s\n",
	      n_elem - 1, v_size, mapFile->fileName);
      raise_error ("LoadMapIndex", Bff, ER_CRITICAL);
    }

    FreeParameter (str);
    str = GetParameter (str, "mapMax=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapIndex", "file5 is not a map file", ER_FAIL);
    StrToVector (str, max);
    FreeParameter (str);

    Show ("\nLoading %s\n", mapFile->fileName);

    if (MapIsFree (*map))
      NewMap (map, size, min, max, dim);
    else if (!IndexAreEquals ((*map)->size, size))
      raise_error ("LoadMapIndex", "incompatible maps", ER_FAIL);

    {
      int DBGmax = 0;
    k = 0;
    ForAllPoints ((*map), (*map)->curPoint) {
      fgets (record, FS_RECORD_SIZE, GetFilePointer (mapFile));
      field = record;
      for (i = 0; i < dim; i++) {
	field += strspn (field, FS_WHITE_SPACES);
	sscanf (field, "%lf", &((*((*map)->curPoint))->values[i]));
	field += strspn (field, "+-.0123456789e");
	/* field += strspn (field, FS_END_OF_FIELD); */
      }
      field += strspn (field, FS_WHITE_SPACES);
      sscanf (field, "%ld", &majIndex[k]);
      DBGmax = (DBGmax < majIndex[k]) ? majIndex[k] : DBGmax;
      k++;
    }
    {
      long DBGclassIndex[16];
      int i, j, cl = 3;
      for (i = 0; i < cl; i++)
	DBGclassIndex[i] = 0;
      for (j = 0; j < k; j++)
	DBGclassIndex[majIndex[j]] ++;
      for (i = 0; i < cl; i++)
	fprintf(stdout,"nombre de points class %d = %ld\n", i, DBGclassIndex[i]);
    }
    fprintf(stdout,"nombre maximun de clase = %d\nk = %ld\n", DBGmax, k);
    }
  }
  Show ("%s\n", " Done.");

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
LoadMapLabel (map_t * map, file_t mapFile, vector_t * label)
{
  mapindex_t      size;
  vector_t        min;
  vector_t        max;
  long            i, k;
  long            dim;
  char           *str = NULL;
  char            record[FS_RECORD_SIZE];
  char           *field;
  long            valeur;

  /*char *begin="Loading "; */

  if (mapFile != NULL) {
    str = GetParameter (str, "mapDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapLabel", "file1 is not a map file", ER_FAIL);
    StrToLong (str, dim);
    NewIndex (&size, dim);
    FreeParameter (str);
    str = GetParameter (str, "mapSize=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapLabel", "file2 is not a map file", ER_FAIL);
    StrToIndex (str, size);
    FreeParameter (str);
    str = GetParameter (str, "pointDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapLabel", "file3 is not a map file", ER_FAIL);
    StrToLong (str, dim);
    NewVector (&min, dim);
    NewVector (&max, dim);
    FreeParameter (str);
    str = GetParameter (str, "mapMin=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapLabel", "file4 is not a map file", ER_FAIL);
    StrToVector (str, min);
    FreeParameter (str);
    str = GetParameter (str, "mapMax=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapLabel", "file5 is not a map file", ER_FAIL);
    StrToVector (str, max);
    FreeParameter (str);

    Show ("\nLoading %s\n", mapFile->fileName);

    if (MapIsFree (*map))
      NewMap (map, size, min, max, dim);
    else if (!IndexAreEquals ((*map)->size, size))
      raise_error ("LoadMapLabel", "incompatible maps", ER_FAIL);

    NewVector (label, (*map)->range);

    k = 0;
    ForAllPoints ((*map), (*map)->curPoint) {
      fgets (record, FS_RECORD_SIZE, GetFilePointer (mapFile));
      field = record;
      for (i = 0; i < dim; i++) {
	field += strspn (field, FS_WHITE_SPACES);
	sscanf (field, "%lf", &((*((*map)->curPoint))->values[i]));
	field += strspn (field, "+-.0123456789e");
	/* field += strspn (field, FS_END_OF_FIELD); */
      }
      field += strspn (field, FS_WHITE_SPACES);
      sscanf (field, "%ld", &(valeur));
      (*label)->values[k] = (double) (valeur);
      k++;
    }
  }
  Show ("%s\n", " Done.");

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
LoadMapProba (map_t * map, file_t mapFile)
{
/* les fichiers d'activation peuvent etre tres longs la donnee
   FS_RECORD_SIZE a ete redimmentionnee a 32768 pour prevoir la
   lecture d'un long enregistrement */

  mapindex_t      size;
  vector_t        min;
  vector_t        max;
  long            i;
  long            dim, dimProba = 1;
  char           *str = NULL;
  char            record[FS_RECORD_SIZE];
  /* char            record[taille]; */
  /* char           *record = NULL; */
  char           *field;

  char           *p;
  int             indexed_file = OFF;

  int n_elem;

  /*char *begin="Loading "; */

  if (mapFile != NULL) {
    p = GetHeader (mapFile);

    if ( *p == '@' )
      indexed_file = ON;

    str = GetParameter (str, "mapDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapProba", "file is not a map file", ER_FAIL);
    StrToLong (str, dim);
    NewIndex (&size, dim);
    FreeParameter (str);

    str = GetParameter (str, "mapSize=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapProba", "file is not a map file", ER_FAIL);
    StrToIndex (str, size);
    FreeParameter (str);

    str = GetParameter (str, "pointDim=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapProba", "file is not a map file", ER_FAIL);
    StrToLong (str, dim);
    FreeParameter (str);

    str = GetParameter (str, "mapMin=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapProba", "file is not a map file", ER_FAIL);

    NewVector (&min, dimProba);
    n_elem = StrToVector(str, min);
    if (n_elem) {
      int v_size = min->dim;
      char Bff[BFFSIZE];
      sprintf(Bff,"incompatible MIN_STRING string and expected vector size: (%d < %d). In file :\n\t *** %s\n\t     Size must be %d ***\n",
	      n_elem - 1, v_size, mapFile->fileName, v_size);
      raise_error ("LoadMap", Bff, ER_CRITICAL);
    }
    FreeParameter (str);

    str = GetParameter (str, "mapMax=", GetHeader (mapFile));
    if (str == NULL)
      raise_error ("LoadMapProba", "file is not a map file", ER_FAIL);

    NewVector (&max, dimProba);
    n_elem = StrToVector(str, max);
    if (n_elem) {
      int v_size = max->dim;
      char Bff[BFFSIZE];
      sprintf(Bff,"incompatible MAX_STRING string and expected vector size: (%d < %d). In file :\n\t *** %s\n",
	      n_elem - 1, v_size, mapFile->fileName);
      raise_error ("LoadMap", Bff, ER_CRITICAL);
    }
    FreeParameter (str);

    Show ("\nLoading %s\n", mapFile->fileName);

/*  if ( MapIsFree(*map) )
   NewMap( map, size, min, max, dim );
   else
   if ( !IndexAreEquals( (*map)->size, size ) )
   raise_error( "LoadMapProba", "incompatible maps", ER_FAIL );  */

    NewMap (map, size, min, max, dim);

    ForAllPoints ((*map), (*map)->curPoint) {
      fgets (record, FS_RECORD_SIZE, GetFilePointer (mapFile));

      field = record;

      if ( indexed_file ) {
	int  n, indj;

	for (i = 0; i < dim; i++)
	  (*((*map)->curPoint))->values[i] = 0.0;

	field += strspn (field, FS_WHITE_SPACES);
	sscanf (field, "%d", &n);
	field += strspn (field, "0123456789");

	for (i = 0; i < n; i++) {
	  field += strspn (field, FS_WHITE_SPACES);
	  sscanf (field, "%d", &indj);
	  field += strspn (field, "0123456789");
	  field += strspn (field, FS_WHITE_SPACES);
	  sscanf (field, "%lf", &((*((*map)->curPoint))->values[indj - 1]));
	  field += strspn (field, "+-.0123456789eE");
	}
      } else {
	for (i = 0; i < dim; i++) {
	  field += strspn (field, FS_WHITE_SPACES);
	  sscanf (field, "%lf", &((*((*map)->curPoint))->values[i]));
	  field += strspn (field, "+-.0123456789");
	  /* if (i < dim - 1) {
	     field += strspn (field, "+-.0123456789");
	     field += strspn (field, FS_END_OF_FIELD);
	     } */
	}
	/*if ((*map)->offset%(int)((*map)->range/10)==0)
	  Show("%s","."); */
      }
    }
  }
  Show ("%s\n", " Done.");

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/
void
SaveMap (map_t map, file_t * mapFile)
{
  static char            str[FS_RECORD_SIZE];
  static char            arg[FS_RECORD_SIZE];
  static char            *arg0 = "# mapDim=";
  static char            *arg2 = "mapSize=";
  static char            *arg3 = "pointDim=";
  static char            *arg4 = "mapMin=";
  static char            *arg5 = "mapMax=";
  register int    i;

  for (i = 0; i < FS_RECORD_SIZE; ++i) {
    str[i] = '\0';
    arg[i] = '\0';
  }

  strcpy(arg,arg0);

  if (!MapIsFree (map) && ((*mapFile) != NULL)) {
    LongToStr (map->size->dim, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg2);
    IndexToStr (map->size, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg3);
    LongToStr (map->min->dim, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg4);
    VectorToStr (map->min, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg5);
    VectorToStr (map->max, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, GetHeader (*mapFile));
    SetHeader (*mapFile, arg);
    fprintf (GetFilePointer (*mapFile), "%s\n", GetHeader (*mapFile));

    ForAllPoints (map, map->curPoint) {
      for (i = 0; i < map->min->dim; i++) {
	fprintf (GetFilePointer (*mapFile), "%f", (*(map->curPoint))->values[i]);
	if (i < map->min->dim - 1)
	  fprintf (GetFilePointer (*mapFile), FS_END_OF_FIELD);
      }
      fprintf (GetFilePointer (*mapFile), FS_END_OF_RECORD);
    }
  }
  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/
void
SaveLabelMap (map_t map, long *label, file_t * mapFile)
{
  char            str[FS_RECORD_SIZE];
  char            arg[FS_RECORD_SIZE] = "# LabelMap  mapDim=";
  char            arg2[FS_BUFFER_SIZE] = "mapSize=";
  char            arg3[FS_BUFFER_SIZE] = "pointDim=";

  /* char            arg4[FS_RECORD_SIZE] = "mapMin=";
     char            arg5[FS_RECORD_SIZE] = "mapMax="; */
  int             i;

  if (!MapIsFree (map) && ((*mapFile) != NULL)) {
    LongToStr (map->size->dim, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg2);
    IndexToStr (map->size, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg3);
    LongToStr (((map->min->dim)), str);
    strcat (arg, str);
    strcat (arg, " ");
    /*    strcat( arg, arg4 );
       VectorToStr( map->min, str );
       strcat( arg, str );
       strcat( arg, " " );
       strcat( arg, arg5 );
       VectorToStr( map->max, str );
       strcat( arg, str );
       strcat( arg, " " ); */
    strcat (arg, GetHeader (*mapFile));

    SetHeader (*mapFile, arg);
    fprintf (GetFilePointer (*mapFile), "%s\n", GetHeader (*mapFile));
    ForAllPoints (map, map->curPoint) {
      for (i = 0; i < map->min->dim; i++) {
	fprintf (GetFilePointer (*mapFile), "%f", (*(map->curPoint))->values[i]);
	fprintf (GetFilePointer (*mapFile), FS_END_OF_FIELD);
      }
      fprintf (GetFilePointer (*mapFile), "%ld", label[map->offset]);
      fprintf (GetFilePointer (*mapFile), FS_END_OF_RECORD);
    }
  }
  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract  cree une carte indexee qui peut etre                         */
/*           reprise par la classification hierarchique CAH               */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/
void
SaveIndexMap (map_t map, long *index, file_t * mapFile, double *membr)
{
  char            str[FS_RECORD_SIZE];
  char            arg[FS_RECORD_SIZE] = "# LabelMap  mapDim=";
  char            arg2[FS_BUFFER_SIZE] = "mapSize=";
  char            arg3[FS_BUFFER_SIZE] = "pointDim=";
  char            arg4[FS_RECORD_SIZE] = "mapMin=";
  char            arg5[FS_RECORD_SIZE] = "mapMax=";
  int             i;

  if (!MapIsFree (map) && ((*mapFile) != NULL)) {
    LongToStr (map->size->dim, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg2);
    IndexToStr (map->size, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg3);
    LongToStr (((map->min->dim)), str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg4);
    VectorToStr (map->min, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg5);
    VectorToStr (map->max, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, GetHeader (*mapFile));

    SetHeader (*mapFile, arg);
    fprintf (GetFilePointer (*mapFile), "%s\n", GetHeader (*mapFile));

    ForAllPoints (map, map->curPoint) {
      for (i = 0; i < map->min->dim; i++) {
	fprintf (GetFilePointer (*mapFile), "%f", (*(map->curPoint))->values[i]);
	fprintf (GetFilePointer (*mapFile), FS_END_OF_FIELD);
      }
      if (membr == NULL || membr[map->offset] != 0)
	fprintf (GetFilePointer (*mapFile), "%ld", index[map->offset]);
      fprintf (GetFilePointer (*mapFile), FS_END_OF_RECORD);
    }
  }
  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract  cree une carte qui peut etre representee par                 */
/*           Sammon (som_pak-3.1)                                         */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/
void
SaveIndexSaMap (map_t map, long *index, file_t * mapFile, char *topol, double *membr)
{
  char            str[FS_RECORD_SIZE];
  char            arg[FS_RECORD_SIZE] = "";

  /* char            arg2[FS_BUFFER_SIZE] = "hexa"; */
  char            argx1[FS_BUFFER_SIZE] = "1";
  char            argx[FS_BUFFER_SIZE];
  char            arg3[FS_BUFFER_SIZE] = "gaussian";
  int             i, d, inv;

  if (!MapIsFree (map) && ((*mapFile) != NULL)) {
    LongToStr (((map->min->dim)), str);
    strcat (arg, str);
    strcat (arg, " ");
/*    strcat( arg, arg2 );  */
    strcat (arg, topol);
    strcat (arg, " ");
    if (map->size->dim == 1) {
      strcat (arg, argx1);
      strcat (arg, " ");
    } else
      for (d = 0; d < map->size->dim; d++) {
	inv = map->size->dim - (d + 1);
	sprintf (argx, "%ld", map->size->values[inv]);
	strcat (arg, argx);
	strcat (arg, " ");
      }

    strcat (arg, arg3);
    strcat (arg, GetHeader (*mapFile));

    SetHeader (*mapFile, arg);
    fprintf (GetFilePointer (*mapFile), "%s\n", GetHeader (*mapFile));
    ForAllPoints (map, map->curPoint) {
      for (i = 0; i < map->min->dim; i++) {
	fprintf (GetFilePointer (*mapFile), "%f", (*(map->curPoint))->values[i]);
	fprintf (GetFilePointer (*mapFile), FS_END_OF_FIELD);
      }
      if (membr == NULL || membr[map->offset] != 0)
	fprintf (GetFilePointer (*mapFile), "%ld", index[map->offset]);
      fprintf (GetFilePointer (*mapFile), FS_END_OF_RECORD);
    }
  }
  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
ScaleMap (map_t map)
{
  int             i;

  ForAllPoints (map, map->curPoint) {
    OffsetToIndex (map->offset, map->index, map->size);
    for (i = 0; i < Min (map->size->dim, (*(map->firstPoint))->dim); i++) {

      (*(map->curPoint))->values[i] =
	(map->max->values[i] -
	 map->min->values[i]) * (double) (map->index->values[i]) /
	(double) (map->size->values[i] - 1) + map->min->values[i];

    }
  }

  return;
}

void
Scale5Map (map_t map, long dim)
{
  int             i, j;

  Show ("\n%s\n", "Scale5Map");
  ForAllPoints (map, map->curPoint) {
    OffsetToIndex (map->offset, map->index, map->size);
    for (i = 0; i < Min (map->size->dim, (*(map->firstPoint))->dim); i++) {

      (*(map->curPoint))->values[i] =
	(map->max->values[i] -
	 map->min->values[i]) * (double) (map->index->values[i]) /
	(double) (map->size->values[i] - 1) + map->min->values[i];

    }
  }

  ForAllPoints (map, map->curPoint) {
    OffsetToIndex (map->offset, map->index, map->size);
    j = 0;
    for (i = Min (map->size->dim, (*(map->firstPoint))->dim); i < dim; i++) {
      (*(map->curPoint))->values[i] = (*(map->curPoint))->values[j];
      if (j == 0)
	j++;
      else
	j = 0;

    }
  }

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

scalar_t
Neighborhood (mapindex_t w, mapindex_t k, scalar_t s0, scalar_t beta)
{
  double          d, dist;
  double          result;

  if (beta == 0.0)
    return (0.0);
  else {
    d = IndexDistance (w, k);
    dist = d / (beta * s0);
    dist *= dist;
    result = exp (-0.5 * dist);

    return (result);
  }
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

scalar_t
K1 (scalar_t u, scalar_t t)
{
  return (exp (-u / t) / t);
}
/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

scalar_t
K (scalar_t u, scalar_t t)
{
  return (exp (-0.5 * Square (u / t)) / (SQRT2PI * t));
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
ClassSom (som_t som)
{
  /*-------------------------------------*/
  /* On trouve la cellule la plus proche */
  /*-------------------------------------*/
  /* debug */
  int             numap = 0;

  som->winnerDistance = -1;
  ForAllPoints (som->map, som->map->curPoint) {

    SubstractVectors (som->distanceVector, som->inputVector, *(som->map->curPoint));
    som->distanceNorm = EuclidNorm (som->distanceVector);
    if (som->distanceNorm < som->winnerDistance || som->winnerDistance == -1) {
      som->winner = som->map->curPoint;
      som->winnerDistance = som->distanceNorm;
    }
    numap++;
  }

  som->winnerOffset = som->winner - som->map->firstPoint;
/* OffsetToIndex( som->winnerOffset, som->winnerIndex, som->map->size ); */
  som->winnerIndex = (*(som->winner))->index;
  som->info->values[SI_WINNEROFFSET] = som->winnerOffset;
  som->info->values[SI_ERROR] = som->winnerDistance;
  som->info->index = som->winnerIndex;

}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
LearnSom (som_t som)
{

/*---------------------------*/
  /* On trouve le representant */
/*---------------------------*/
  ClassSom (som);

/*---------------------------------*/
  /* On adapte les poids de la carte */
/*---------------------------------*/

  LearnAdaptSom(som);
}

void
LearnAdaptSom (som_t som)
{
  long            i;
/*  som->smoothDistance = som->map->size->values[0]*som->learningRate;
 */

  ForAllPoints (som->map, som->map->curPoint) {
    /*OffsetToIndex( som->map->offset, som->map->index, som->map->size ); */
    som->map->index = (*(som->map->curPoint))->index;
/* meilleure topographie de la carte avec Kp2 plutot que avec Kp */
    som->neighborhood =
      Kp2 (IndexDistance (som->winnerIndex, som->map->index), som->smoothDistance);
    if ((som->neighborhood) > som->threshold_neighborhood) {
      SubstractVectors (som->distanceVector, som->inputVector, *(som->map->curPoint));
      for (i = 0; i < (*(som->map->curPoint))->dim; i++) {
	(*(som->map->curPoint))->values[i] +=
	  som->learningRate * som->neighborhood * som->distanceVector->values[i];
      }
    }
  }
  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
NewSom (som_t * som, mapindex_t mapSize, vector_t mapMin, vector_t mapMax, long inputDim)
{
  *som = (som_t) malloc (sizeof (somDesc_t));

  if (*som == NULL)
    raise_error ("NewSom", "malloc failed", ER_FAIL);

  NewMap (&((*som)->map), mapSize, mapMin, mapMax, inputDim);

  (*som)->inputVector            = NULL;
  (*som)->info                   = NULL;
  (*som)->winner                 = NULL;
  (*som)->winnerOffset           = 0;
  NewIndex (&((*som)->winnerIndex), mapSize->dim);
  (*som)->winnerDistance         = 0.0;
  NewVector (&((*som)->distanceVector), inputDim);
  (*som)->distanceNorm           = 0.0;
  (*som)->neighborhood           = 0.0;
  (*som)->threshold_neighborhood = SomThresholdNeighborhood;
  (*som)->learningRate           = 0.0;
  (*som)->learningRateMax        = 0.0;
  (*som)->learningRateMin        = 0.0;
  (*som)->smoothDistance         = 0.0;
  (*som)->smoothDistanceMax      = 0.0;
  (*som)->smoothDistanceMin      = 0.0;
  (*som)->cycle                  = 0;
  (*som)->nbCycle                = 0;
  (*som)->freqTest               = 0;
  (*som)->learnSet               = NULL;
  (*som)->learnSetInfo           = NULL;
  (*som)->testSet                = NULL;
  (*som)->testSetInfo            = NULL;
  (*som)->tempStochastic         = OFF;
  (*som)->appBatch               = OFF;

  ScaleMap ((*som)->map);

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
FreeSom (som_t * som)
{
  if (!SetIsFree ((*som)->learnSet))
    FreeSet (&((*som)->learnSet));
  if (!SetIsFree ((*som)->learnSetInfo))
    FreeSet (&((*som)->learnSetInfo));
  if (!SetIsFree ((*som)->testSet))
    FreeSet (&((*som)->testSet));
  if (!SetIsFree ((*som)->testSetInfo))
    FreeSet (&((*som)->testSetInfo));
  FreeVector (&((*som)->distanceVector));
  FreeIndex (&((*som)->winnerIndex));
  FreeMap (&((*som)->map));
  free ((*som));
  *som = NULL;

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
LoadSom (som_t * som, file_t file)
{
  *som = (som_t) malloc (sizeof (somDesc_t));

  if (*som == NULL)
    raise_error ("LoadSom", "malloc failed", ER_FAIL);

  LoadMap (&((*som)->map), file);

  (*som)->inputVector        = NULL;
  (*som)->info               = NULL;
  (*som)->winner             = NULL;
  (*som)->winnerOffset       = 0;
  NewIndex (&((*som)->winnerIndex), (*som)->map->size->dim);
  (*som)->winnerDistance     = 0.0;
  NewVector (&((*som)->distanceVector), (*som)->map->min->dim);
  (*som)->distanceNorm       = 0.0;
  (*som)->neighborhood       = 0.0;
  (*som)->learningRate       = 0.0;
  (*som)->learningRateMax    = 0.0;
  (*som)->learningRateMin    = 0.0;
  (*som)->smoothDistance     = 0.0;
  (*som)->smoothDistanceMax  = 0.0;
  (*som)->smoothDistanceMin  = 0.0;
  (*som)->cycle              = 0;
  (*som)->nbCycle            = 0;
  (*som)->freqTest           = 0;
  (*som)->learnSet           = NULL;
  (*som)->learnSetInfo       = NULL;
  (*som)->testSet            = NULL;
  (*som)->testSetInfo        = NULL;
  (*som)->tempStochastic     = OFF;
  (*som)->appBatch           = OFF;

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
CycleSom (som_t som)
{
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  /* APPRENTISSAGE                                                        */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  if ( ! som->tempStochastic ) {
    /* Si tempereture ne pas en mode stochastique: Alors on calcule le
       taux d'apprentissage et la distance 'smooth' a chaque cycle uniquement */
    som->learningRate =
      som->learningRateMax * pow (som->learningRateMin / som->learningRateMax,
				    (double) (som->cycle) / (double) (som->nbCycle - 1));
    som->smoothDistance =
      som->smoothDistanceMax * pow (som->smoothDistanceMin / som->smoothDistanceMax,
				    (double) (som->cycle) / (double) (som->nbCycle - 1));
  }

  /*--- Pour tous les echantillons du jeu d'apprentissage */
  ForAllElements (som->learnSet, som->learnSet->curElement) {

    if ( som->tempStochastic ) {
      /* Si tempereture en mode stochastique: Alors on calcule le taux
         d'apprentissage et la distance 'smooth' a chaque iteration */
      som->learningRate =
	som->learningRateMax * pow (som->learningRateMin / som->learningRateMax,
				    (som->step / (som->stepMax - 1)));

      /*--- on calcule la temperature pour l'iteration */
      som->smoothDistance =
	som->smoothDistanceMax * pow (som->smoothDistanceMin / som->smoothDistanceMax,
				      (som->step / (som->stepMax - 1)));
    }

#ifdef DEBUG
    if ( som->tempStochastic ) {
      if ( ! som->learnSet->index ) {
	fprintf(stdout,"DEBUG(CycleSom): %8ld  %10.6f %10.6f %10.6f %12.0f %12.0f",
		som->learnSet->index, som->learningRate, som->learningRateMin,
		som->learningRateMax, som->step, som->stepMax);
	fprintf(stdout,"   %10.6f %10.6f %10.6f\n", som->smoothDistance,
		som->smoothDistanceMin, som->smoothDistanceMax);
	fflush(stdout);
      }
    } else {
      if ( ! som->learnSet->index ) {
	fprintf(stdout,"DEBUG(CycleSom): %8ld  %10.6f %10.6f %10.6f %12s %12s",
		som->learnSet->index, som->learningRate, som->learningRateMin,
		som->learningRateMax, "---", "---");
	fprintf(stdout,"   %10.6f %10.6f %10.6f\n", som->smoothDistance,
		som->smoothDistanceMin, som->smoothDistanceMax);
	fflush(stdout);
      }
    }
#endif

    /*--- on presente l'echantillon courant a l'entree du reseau */
    som->info = *(som->learnSetInfo->firstElement + som->learnSet->index);
    som->inputVector = *(som->learnSet->curElement);

    /*--- on adapte les poids de la carte a l'echantillon courant */
    /* pour apprentissage stochastique on appelle LearSom
       pour Batch ClassSom
    */
    if ( !som->appBatch )
      LearnSom (som);

    (som->step)++;
  }

  if ( som->appBatch )
    LearnAdaptSom (som);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  /* STATISTIQUES                                                          */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
  
  if ((som->testSet) != (som->learnSet)) {
    ForAllElements (som->testSet, som->testSet->curElement) {
      /*--- on presente l'echantillon courant a l'entree du reseau */
      som->info = *(som->testSetInfo->firstElement + som->testSet->index);
      som->inputVector = *(som->testSet->curElement);
      
      /*--- on trouve le representant de l'echantillon */
      ClassSom (som);
    }
  } 
  SomStat (som);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
SomStat (som_t som)
{
  /* file_t          somFile;
     file_t          setFile; */

  Rms (som->testSetInfo, SI_ERROR);

  if (som->testSetInfo->stat->values[SET_RMS] < som->testSetInfo->stat->values[SET_BESTRMS]) {
/*
    NewFile( &somFile, "best-rms.som", FS_OUTPUT_MODE, "" );
    SaveMap( som->map, somFile );
    FreeFile( &somFile );
    NewFile( &setFile, "best-rms.info", FS_OUTPUT_MODE, "" );
    SaveSet( som->learnSetInfo, setFile );
    FreeFile( &setFile );
    */
    som->testSetInfo->stat->values[SET_BESTRMS] = som->testSetInfo->stat->values[SET_RMS];
  }

  Contrast (som->testSetInfo, som->map);

  if (som->testSetInfo->stat->values[SET_CONTRAST] <
      som->testSetInfo->stat->values[SET_BESTCONTRAST]) {
/*
     NewFile( &somFile, "best-contrast.som", FS_OUTPUT_MODE, "" );
     SaveMap( som->map, somFile );
     FreeFile( &somFile );
     NewFile( &setFile, "best-contrast.info", FS_OUTPUT_MODE, "" );
     SaveSet( som->learnSetInfo, setFile );
     FreeFile( &setFile );
     */
    som->testSetInfo->stat->values[SET_BESTCONTRAST] =
      som->testSetInfo->stat->values[SET_CONTRAST];
  }

  if (!som->freqTest || !((som->cycle+1) % som->freqTest)) {
    Show ("cycle %ld : ", som->cycle + 1);
    Show ("h=%f, ", som->smoothDistance);
    Show ("rms=%f, ", som->testSetInfo->stat->values[SET_RMS]);
    Show ("alp=%f, ", som->learningRate);
    Show ("contrast=%f\n", som->testSetInfo->stat->values[SET_CONTRAST]);
  }
}

/*========================================================================*/
/*                  InitSom                                               */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*          void  (but modify som->map)                                   */
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/* Initialization of the map. All the prototype are randomly drawn        */
/*   from a ltidimensional Gaussian Distribution with mean vector       */
/*   , the gravity center of the set and variance matrix sigma          */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
InitSom (som_t * som, set_t set)
{
  int             i;
  vector_t        mu;			       /*Mean Vector */

/* vector_t        sigma; *//* Variance simga11, sigma22 .... */
  /* vector_t        diff; */
/* double          initval; *//* Initial coordinate of a map prototype */
  double         *a;
  double          normA, sumA, k, r;

  /* double          b = 0.5; */

  map_t           map;

  /* Initialisation of the Mean and Variance */
  NewVector (&mu, set->dim);

  /*
     for (i=0;i<(set->dim);i++) SetVectorValue(mu,i,0.0);
   */
  /*
     NewVector(&sigma,set->dim);
     for (i=0;i<(set->dim);i++) SetVectorValue(sigma,i,0.0);
     NewVector(&diff,set->dim);
   */

  /* Computation of the Mean */
  ForAllElements (set, set->curElement) {
    mu = AddVectors (mu, mu, (*(set->curElement)));
  }

  for (i = 0; i < (set->dim); i++) {
    SetVectorValue (mu, i, mu->values[(i)] / (set->size));
  }

  /* Computation of the variances  sigma */
  /*
     ForAllElements( set,set->curElement) {
     diff = SubstractVectors(diff,(*(set->curElement)), );
     for (i=0;i<(set->dim);i++) SetVectorValue(diff,i,Square(diff->values[(i)]));
     sigma=AddVectors(sigma,sigma,diff);
     }
     for (i=0;i<(set->dim);i++) {
     SetVectorValue(sigma,i,(sigma->values[(i)]/((set->size)-1)));
     }
   */

  map = (*som)->map;

/* pour chaque point de la carte, en dimension 5, generation de 6 nombres aleatoires
   5 alpha et un k, tous compris entre 0 et 1.
   le vecteur A = (a1, a2, a3, a4, a5)
   || A || = sqrt (a1 carre + a2 carre + a3 carre + a4 carre + a5 carre)
   le vecteur U = ( a1 / ||A|| , a2 / ||A|| , a3 / ||A||, a4 / ||A||, a5 / ||A|| )
   Dans chaque direction, la valeur du referent =
   la composante du vecteur mu + la composante du vecteur U
 */

  a = (double *) malloc (set->dim * sizeof (double));

  if (a == NULL)
    raise_error ("InitSom", "malloc failed", ER_FAIL);

  ForAllPoints (map, map->curPoint) {
    sumA = 0;
    for (i = 0; i < set->dim; i++) {
      r = rand ();
      a[i] = (r / (RAND_MAX + 1.0));
      sumA += pow (a[i], 2);
    }

    normA = sqrt (sumA);

    k = (rand () / (RAND_MAX + 1.0));

    for (i = 0; i < set->dim; i++) {
      a[i] /= normA;
      a[i] *= 0.1 * k;
    }

    /* OffsetToIndex( map->offset, map->index, map->size ); */
    for (i = 0; i < set->dim; i++)
      (*(map->curPoint))->values[i] = a[i] + mu->values[i];

  }

  FreeVector (&mu);
  free (a);

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

scalar_t
Rms (set_t set, int errorIndex)
{
  set->stat->values[SET_RMS] = 0.0;
  ForAllElements (set, set->curElement)
    set->stat->values[SET_RMS] += Square ((*(set->curElement))->values[errorIndex]);
  set->stat->values[SET_RMS] = sqrt (set->stat->values[SET_RMS] / set->size);

  return (set->stat->values[SET_RMS]);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/
scalar_t
Contrast (set_t set, map_t map)
{
  vector_t        card;
  scalar_t        Card, contrast, x;
  long            i;

  Card = (double) set->size / (double) map->range;

  NewVector (&card, map->range);

  ForAllElements (set, set->curElement)
    card->values[(long) (*(set->curElement))->values[SI_WINNEROFFSET]]++;

  contrast = 0.0;
  for (i = 0; i < card->dim; i++) {
    x = card->values[i] / Card - 1.0;
    contrast += Square (x);
  }

  contrast = sqrt (contrast / (double) map->range);
  set->stat->values[SET_CONTRAST] = contrast;

  FreeVector (&card);

  return (contrast);
}

/**************************************************************************/
/* Author : Philippe DAIGREMONT                                           */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract : bibliotheque de fonction PSOM                               */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
/* Parameters : neant                                                     */
/**************************************************************************/

/*========================================================================*/
/*                  InitPsom                                              */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*          void  (but modify psom->som->map   )                          */
/*                (           psom->dev        )                          */
/*                (           psom->activation )                          */
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/* Initialization of the map. All the prototype are randomly drawn        */
/*   from a ltidimensional Gaussian Distribution with mean vector       */
/*   , the gravity center of the set and variance matrix sigma          */
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
InitPsom (psom_t * psom, set_t set)
{
  scalar_t       *matrix;
  map_t           map;

  mapDesc_t       window;

  map = (*psom)->som->map;
  window.firstPoint = map->firstPoint;
  window.range = map->range;
  NewIndex (&(window.index), map->index->dim);

  matrix = (*psom)->mapdistance->values;

  /*  Initialisation of the map of mean  */
  /*  InitSom( &((*psom)->som),set);     */

  /*  Initialisation of the map of deviation  */
  /*  deplacee dans Load2Psom pour ne pas effectuer une carte de variances
     eventuellement reprise d'une experience precedente   */
  /* ScaleMap( (*psom)->dev );             */

  /*  Initialisation of the map of activation */
  ScaleMap ((*psom)->activation);

  /* Initialisation of the map distance matrix */
  ForAllPoints (map, map->curPoint) {
    /*   OffsetToIndex( map->offset, map->index, map->size );   */
    map->index = (*(map->curPoint))->index;
    ForAllPoints (&window, window.curPoint) {
      OffsetToIndex (window.offset, window.index, map->size);
      matrix[window.offset * (window.range) + (map->offset)] =
	IndexDistance (window.index, map->index);
      matrix[(map->offset) * (map->range) + (window.offset)] =
	matrix[window.offset * (window.range) + (map->offset)];
    }

  }

  return;
}

/*========================================================================*/
/*                 Kp                                                     */
/*-Parameters-------------------------------------------------------------*/
/*    scalar_t u     : input variable                                     */
/*    scalar_t t     : variance                                           */
/*-Return-----------------------------------------------------------------*/
/*             exp(-0.5*Square(u/t))                                      */
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/
scalar_t
Kp (scalar_t u, scalar_t t)
{
  return (exp (-0.5 * Square (u / t)));
}

scalar_t
Kp2 (scalar_t u, scalar_t t)
{
  return (exp (-0.5 * (u / t)));
}

/*========================================================================*/
/*                      Rbf1                                              */
/*-Parameters-------------------------------------------------------------*/
/*              vector_t z     :  mean                                    */
/*              vector_t w     :  input variable                          */
/*              scalar_t sigma :  variance                                */
/*-Return-----------------------------------------------------------------*/
/*                 f(w) =                                                 */
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

scalar_t
Rbf1 (vector_t z, vector_t w, scalar_t sigma)
{
  double          sum = 0.0;
  long            i;

/*--------------------------------------*/
  /* on se protege des divisions par zero */
/*--------------------------------------*/
  sigma = (sigma == 0.0 ? ZEROPLUS : sigma);

  for (i = 0; i < z->dim; i++)
    sum += Square (w->values[i] - z->values[i]);

  sum /= Square (sigma);

  return (exp (-0.5 * sum) / (pow (sigma * SQRT2PI, (double) z->dim)));
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

scalar_t
Rbf (scalar_t d, scalar_t sigma, scalar_t dim)
{
/*--------------------------------------*/
  /* on se protege des divisions par zero */
/*--------------------------------------*/
  sigma = (sigma == 0.0 ? ZEROPLUS : sigma);

  return (exp (-0.5 * (d / Square (sigma))) / (pow (sigma * SQRT2PI, dim)));
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

scalar_t
Bell (vector_t z, vector_t w, scalar_t sigma)
{
  double          sum = 0.0;
  long            i;

/*--------------------------------------*/
  /* on se protege des divisions par zero */
/*--------------------------------------*/
  sigma = (sigma == 0.0 ? 0.000001 : sigma);

  for (i = 0; i < z->dim; i++)
    sum += Square (w->values[i] - z->values[i]);

  sum /= Square (RBF_TEMPERATURE * sigma);

  return (exp (-0.5 * sum));
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
ClassPsom (psom_t psom)
{
  double          qd;
  som_t           som;
  scalar_t        sigma;

  som = psom->som;

  som->winnerDistance = 0;
/*-------------------------------------*/
  /* On trouve la cellule la plus proche */
/*-------------------------------------*/
  ForAllPoints (som->map, som->map->curPoint) {
    qd = QuadraticDistance (som->inputVector, (*(som->map->curPoint)));
    /* Cherchons la variance de la classe (cellule) courante */
    psom->dev->curPoint = psom->dev->firstPoint + som->map->offset;
    sigma = (*(psom->dev->curPoint))->values[0];

    som->distanceNorm = Rbf (qd, sigma, (scalar_t) som->inputVector->dim);

    if (som->distanceNorm > som->winnerDistance || som->winnerDistance == 0) {
      som->winner = som->map->curPoint;
      som->winnerDistance = som->distanceNorm;
    }
  }

  som->winnerOffset = som->winner - som->map->firstPoint;
/*OffsetToIndex( som->winnerOffset, som->winnerIndex, som->map->size ); */
  som->winnerIndex = (*(som->winner))->index;
  som->info->values[SI_WINNEROFFSET] = som->winnerOffset;
  som->info->values[SI_DENSITY] = som->winnerDistance;
  som->info->index = som->winnerIndex;

  SubstractVectors (som->distanceVector, som->inputVector, *(som->winner));
  som->distanceNorm = EuclidNorm (som->distanceVector);
  som->info->values[SI_ERROR] = som->distanceNorm;

}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

/*========================================================================*/
/* Function   :                                                           */
/* Parameters :                                                           */
/*                                                                        */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
void
NewPsom (psom_t * psom, mapindex_t mapSize, vector_t mapMin, vector_t mapMax, long inputDim)
{
  /*map_t map; */
  long            unitNumber;
  vector_t        init;

  *psom = (psom_t) malloc (sizeof (psomDesc_t));

  if (*psom == NULL)
    raise_error ("NewPsom", "malloc failed", ER_FAIL);

  NewSom (&((*psom)->som), mapSize, mapMin, mapMax, inputDim);

/*  map = (*psom)->som->map;
   SubstractVectors( (*psom)->som->distanceVector, map->max, map->min );
   (*psom)->som->distanceNorm = EuclidNorm( (*psom)->som->distanceVector );
 */
  NewVector (&init, 1);
  init->values[0] = 1.0;

  NewMap (&((*psom)->dev), mapSize, init, init, 1);

  NewMap (&((*psom)->activation), mapSize, init, init, 1);

  unitNumber = (*psom)->som->map->range;
  NewMatrix (&(*psom)->mapdistance, unitNumber, unitNumber);

  FreeVector (&init);
  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

/*========================================================================*/
/* Function   :                                                           */
/* Parameters :                                                           */
/*                                                                        */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
void
FreePsom (psom_t * psom)
{

  FreeMap (&((*psom)->activation));
  FreeMap (&((*psom)->dev));
  FreeSom (&((*psom)->som));
  FreeMatrix (&((*psom)->mapdistance));
  free ((*psom));
  *psom = NULL;

  return;
}

/*========================================================================*/
/* Function   :                                                           */
/* Parameters :                                                           */
/*                                                                        */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
LoadPsom (psom_t * psom, file_t mapFile, file_t devFile)
{
  *psom = (psom_t) malloc (sizeof (psomDesc_t));

  if (*psom == NULL)
    raise_error ("LoadPsom", "malloc failed", ER_FAIL);

  LoadSom (&((*psom)->som), mapFile);
  LoadMap (&((*psom)->dev), devFile);
  LoadMap (&((*psom)->activation), devFile);
  return;
}

/* chargement de la carte des poids issue du som */

void
Load2Psom (psom_t * psom, file_t mapFile, file_t devFile)
{
  long            unitNumber;
  vector_t        init;

  *psom = (psom_t) malloc (sizeof (psomDesc_t));

  if (*psom == NULL)
    raise_error ("Load2Psom", "malloc failed", ER_FAIL);

  LoadSom (&((*psom)->som), mapFile);

  NewVector (&init, 1);
  init->values[0] = 1.0;

  if (devFile == NULL) {
    NewMap (&((*psom)->dev), (*psom)->som->map->size, init, init, 1);
    ScaleMap ((*psom)->dev);
  } else
    LoadMap (&((*psom)->dev), devFile);

  NewMap (&((*psom)->activation), (*psom)->som->map->size, init, init, 1);

  unitNumber = (*psom)->som->map->range;
  NewMatrix (&(*psom)->mapdistance, unitNumber, unitNumber);

  FreeVector (&init);

  return;
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
CyclePsom (psom_t psom)
{
  /*  file_t f; */
  scalar_t        sigma;

  /* scalar_t        qd; */
  double          numerateurS, denominateur, d;

  /*  scalar_t SumOfLine; */
  vector_t        numerateurW;

/*  matrix_t Kvois; *//* Matrice des distance sur la cartes transforme par la fonction                             de voisinage */

  long            i;

  /* long            j; */
  som_t           som;
  map_t           map;

  som = psom->som;
  map = som->map;

  if (psom->VarieSmoothD == ON)
    som->smoothDistance =
      som->smoothDistanceMax * pow (som->smoothDistanceMin / som->smoothDistanceMax,
				    (double) (som->cycle) / (double) (som->nbCycle - 1));
  else
    som->smoothDistance = som->smoothDistanceMax;

  /*----------------------------------------*/
  /* Mise a jour de la matrice de voisinage */
  /*----------------------------------------*/
  /*
     NewMatrix( &Kvois, map->range, map->range);
     for (i=0; i< (map->range);i++) {
     v    SumOfLine=0;
     for (j=0; j< (map->range);j++) {
     Kvois->values[i*(map->range)+j]=Kp(psom->mapdistance->values[i*(map->range)+j] , som->smoothDistance );
     SumOfLine +=  Kvois->values[i*(map->range)+j];
     }
     for (j=0; j< (map->range);j++) {
     Kvois->values[i*(map->range)+j] /= SumOfLine;
     }
     }
   */
  /*----------------------------------------------------------------------*/
  /* pour tous les echantillons du jeu d'apprentissage                    */
  /* calculer les informations de classement SI_WINNEROFFSET et SI_DENSITY */
  /*----------------------------------------------------------------------*/
  ForAllElements (som->learnSet, som->learnSet->curElement) {
    /*--- on presente l'echantillon courant a l'entree du reseau */
    som->info = *(som->learnSetInfo->firstElement + som->learnSet->index);
    som->inputVector = *(som->learnSet->curElement);

    ClassPsom (psom);
  }

  if ((som->testSet) != (som->learnSet)) {
    ForAllElements (som->testSet, som->testSet->curElement) {
      /*--- on presente l'echantillon courant a l'entree du reseau */
      som->info = *(som->testSetInfo->firstElement + som->testSet->index);
      som->inputVector = *(som->testSet->curElement);
      ClassPsom (psom);
    }
  }
  /*
     NewFile( &f, "debug.info", FS_OUTPUT_MODE, "" );
     SaveSet( som->learnSetInfo, f );
     FreeFile( &f );
   */

  /*------------------------*/
  /* pour tous les neurones (classes) */
  /*------------------------*/
  ForAllPoints (map, map->curPoint) {
    denominateur = 0.0;
    numerateurS = 0.0;			       /* Numerateur pour la nouvelle variance de la classe courante */
                  			       /* Numerateur du Nouveau vecteur moyenne de la classe courante */

    NewVector (&numerateurW, (*(map->firstPoint))->dim);

    /* on se positionne sur la bonne variance pour la classe courante */
    psom->dev->curPoint = psom->dev->firstPoint + map->offset;
    sigma = (*(psom->dev->curPoint))->values[0];

    /*  Index sur la carte de la classe courante */
    /*OffsetToIndex( map->offset, map->index, map->size ); */
    map->index = (*(map->curPoint))->index;
    /*---------------------------------------------------*/
    /* pour tous les echantillons du jeu d'apprentissage */
    /*---------------------------------------------------*/
    ForAllElements (som->learnSet, som->learnSet->curElement) {
      /*On se positionne sur les bonnes infos concernant l'individu courant */
      som->info = *(som->learnSetInfo->firstElement + som->learnSet->index);
      som->inputVector = *(som->learnSet->curElement);

      /* Index sur la carte du winner pour l'individu courant */
      /*   OffsetToIndex( (long) som->info->values[SI_WINNEROFFSET], som->winnerIndex,  map->size );  */
      som->winnerIndex = som->info->index;
      som->winnerDistance = som->info->values[SI_DENSITY];

      d = psom->mapdistance->values[(int) ((som->info->values[SI_WINNEROFFSET]) * (map->range) +
					   (map->offset))];

      /*      qd = QuadraticDistance( som->inputVector, (*(map->curPoint)) ); */
      /* if (d<=3.0*som->smoothDistance)  */
      if (d <= som->smoothDistance) {
	/* som->neighborhood = Kp(d,som->smoothDistance);    */
	som->neighborhood = Kp2 (d, som->smoothDistance);

	/*
	   Kvois->values[ (int)((som->info->values[SI_WINNEROFFSET]) * (map->range) + (map->offset))];
	 */
	som->distanceNorm = Square (som->info->values[SI_ERROR]);

	denominateur += som->neighborhood;

	for (i = 0; i < som->inputVector->dim; i++) {
	  numerateurW->values[i] += som->neighborhood * som->inputVector->values[i];
	}

	numerateurS += som->neighborhood * som->distanceNorm;
      }
    }					       /* fin de  ForAllElements */

    /* Mise a jour  de la variance */
    /* if ((psom->ComputeVariance==ON)&& (som->smoothDistance < 0.5 ))  */
    if (denominateur > 0.0 && numerateurS > 0.0)
      if (psom->ComputeVariance == ON)
	(*(psom->dev->curPoint))->values[0] =
	  sqrt (numerateurS / (denominateur * som->inputVector->dim));

    /* Mise a jour de la moyenne */
    if (psom->ComputeMoyenne == ON)
      for (i = 0; i < som->inputVector->dim; i++)
	if (denominateur > 0.0)
	  (*(map->curPoint))->values[i] = numerateurW->values[i] / denominateur;

    FreeVector (&numerateurW);
    /*FreeMatrix( &Kvois ); */
  }

  PsomStat (psom);
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
PsomStat (psom_t psom)
{
  som_t           som;

  /* file_t          devFile; */

  som = psom->som;

  SomStat (som);

  /* Si quelque chose s'est ameliore, alors gardons la trace de la carte des variances */
/*if ( som->testSetInfo->stat->values[SET_RMS] == som->testSetInfo->stat->values[SET_BESTRMS] )
   {
   NewFile( &devFile, "best-rms.dev", FS_OUTPUT_MODE, "" );
   SaveMap( psom->dev, devFile );
   FreeFile( &devFile );
   }
 */
/*  if ( som->testSetInfo->stat->values[SET_CONTRAST] == som->testSetInfo->stat->values[SET_BESTCONTRAST] )
   {
   NewFile( &devFile, "best-contrast.dev", FS_OUTPUT_MODE, "" );
   SaveMap( psom->dev, devFile );
   FreeFile( &devFile );
   }
 */
}

/*========================================================================*/
/*                                                                        */
/*-Parameters-------------------------------------------------------------*/
/*-Return-----------------------------------------------------------------*/
/*-Examples---------------------------------------------------------------*/
/*-Abstract---------------------------------------------------------------*/
/*-Dependencies-----------------------------------------------------------*/
/* call     :                                                             */
/* call by  :                                                             */
/*========================================================================*/

void
SaveActivation (psom_t psom, file_t resFile)
{
/* fonction utilisee pour RBF */
  som_t           som;
  map_t           map;

  char            str[FS_RECORD_SIZE];
  char            arg[FS_RECORD_SIZE] = "# mapDim=1";
  char            arg2[FS_BUFFER_SIZE] = "mapSize=";
  char            arg3[FS_BUFFER_SIZE] = "pointDim=";
  char            arg4[FS_BUFFER_SIZE] = "mapMin=";
  char            arg5[FS_BUFFER_SIZE] = "mapMax=";

  som = psom->som;
  map = som->map;

  /*fprintf( GetFilePointer(resFile), ".MAT 2 %d %d\n", som->testSet->size, map->range ); */

  if (!MapIsFree (map) && (resFile != NULL)) {
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg2);
    LongToStr (som->testSet->size, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg3);
    LongToStr (map->range, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg4);
    VectorToStr (map->min, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg5);
    VectorToStr (map->max, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, GetHeader (resFile));
    SetHeader (resFile, arg);
    fprintf (GetFilePointer (resFile), "%s\n", GetHeader (resFile));

    ForAllElements (som->testSet, som->testSet->curElement) {
/*--- on presente l'echantillon courant a l'entree du reseau */
      som->info = *(som->testSetInfo->firstElement + som->testSet->index);
      som->inputVector = *(som->testSet->curElement);
      ForAllPoints (map, map->curPoint) {
	psom->dev->curPoint = psom->dev->firstPoint + map->offset;
	psom->activation->curPoint = psom->activation->firstPoint + map->offset;
	(*(psom->activation->curPoint))->values[0] =
	  Bell (som->inputVector, (*(map->curPoint)), (*(psom->dev->curPoint))->values[0]);
      }

      ForAllPoints (psom->activation, psom->activation->curPoint) {
	fprintf (GetFilePointer (resFile), "%f", (*(psom->activation->curPoint))->values[0]);
	if (psom->activation->offset < psom->activation->range - 1)
	  fprintf (GetFilePointer (resFile), FS_END_OF_FIELD);
      }
      fprintf (GetFilePointer (resFile), FS_END_OF_RECORD);
    }
  }
}

void
SaveProbability (psom_t psom, file_t resFile)
{
/* fonction utilisee pour etablir les probabilites d'appartenance a une classe */
  register int    i;
  som_t           som;
  map_t           map;
  double          qd, referActiv;
  double          proba;
  register int    nb_elem, index_elem;

  /* double          ecart; */
  scalar_t        sigma;

  char            str[FS_RECORD_SIZE];
  char            arg[FS_RECORD_SIZE] = "@ mapDim=";  /* ATTENTION
  fichier qui debute avec '@' signifie fichier en format indexe: c-a-d
  seulement les elements differtents de zero (ou plus grands qu'un
  seuil) seron conserves. La valeur est sauvee precede de l'index de
  la colonne. Si le fichier debute avec '#' signifie format normal:
  toutes les valeures sont sauvegardees */

  char            arg2[FS_BUFFER_SIZE] = "mapSize=";
  char            arg3[FS_BUFFER_SIZE] = "pointDim=";
  char            arg4[FS_BUFFER_SIZE] = "mapMin=";
  char            arg5[FS_BUFFER_SIZE] = "mapMax=";
  long            dim, dimProba = 1;

  double          SeuilProbaMin = SEUILPROBAMIN;
  vector_t        min;
  vector_t        max;

  som = psom->som;
  map = som->map;

  /*fprintf( GetFilePointer(resFile), ".MAT 2 %d %d\n", som->testSet->size, map->range ); */

  if (!MapIsFree (map) && (resFile != NULL)) {
    
    LongToStr (dimProba, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, arg2);
    LongToStr (som->testSet->size, str);
    strcat (arg, str);
    strcat (arg, " ");

    dim = IndexRange (map->size);

    strcat (arg, arg3);
    LongToStr (dim, str);
    strcat (arg, str);
    strcat (arg, " ");

    NewVector (&min, dimProba);
    for (i = 0; i < min->dim; ++i)
      min->values[i] = map->min->values[i];

    strcat (arg, arg4);
    VectorToStr (min, str);
    strcat (arg, str);
    strcat (arg, " ");

    NewVector (&max, dimProba);
    for (i = 0; i < max->dim; ++i)
      max->values[i] = map->max->values[i];

    strcat (arg, arg5);
    VectorToStr (max, str);
    strcat (arg, str);
    strcat (arg, " ");
    strcat (arg, GetHeader (resFile));
    SetHeader (resFile, arg);
    fprintf (GetFilePointer (resFile), "%s\n", GetHeader (resFile));

    /* Pour chaque forme, on etablit la valeur de son activation dans
       chaque referent, c'est a dire la fonction densite : calcul de
       Rbf dans refActiv.  */

    ForAllElements (som->testSet, som->testSet->curElement) {
      /* on presente l'echantillon courant a l'entree du reseau */
      som->info = *(som->testSetInfo->firstElement + som->testSet->index);
      som->inputVector = *(som->testSet->curElement);
      ForAllPoints (map, map->curPoint) {
	psom->dev->curPoint = psom->dev->firstPoint + map->offset;
	psom->activation->curPoint = psom->activation->firstPoint + map->offset;

	qd = QuadraticDistance (som->inputVector, (*(map->curPoint)));
	sigma = (*(psom->dev->curPoint))->values[0];
	referActiv = Rbf (qd, sigma, (scalar_t) som->inputVector->dim);

	(*(psom->activation->curPoint))->values[0] = referActiv;

      }

      /* compte le nombre d'elements differents de zero (superieurs au
         seuil) */
      nb_elem = 0;
      ForAllPoints (psom->activation, psom->activation->curPoint)
	if ( (*(psom->activation->curPoint))->values[0] > SeuilProbaMin )
	  nb_elem ++;

      fprintf (GetFilePointer (resFile), "%d", nb_elem);

      /* sauvegarde uniquement les elements differents de zero avec
         son indice de colonne [1 .. p] */
      index_elem = 0;
      ForAllPoints (psom->activation, psom->activation->curPoint) {
	index_elem ++;
	proba = (*(psom->activation->curPoint))->values[0];
	if ( proba > SeuilProbaMin )
	  fprintf (GetFilePointer (resFile), "%s%d %e", FS_END_OF_FIELD, index_elem, proba);
      }
      fprintf (GetFilePointer (resFile), FS_END_OF_RECORD);
    }
  }
}
