
/*
 */

#define BFFSIZE 1024
#define NONDEFINI_MSG "NON DEFINI"
#define ENABLED_MSG   "<ENABLED>"
#define DISABLED_MSG  "<DISABLED>"


/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :     old file.h                                                */
/**************************************************************************/
/* Abstract :   type, define and proto for files processing   */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_FILE__
#define __SPOUTNIK_FILE__

#define SEUILPROBAMIN      1.0e-8

#define FS_HEADER_SIZE      32768
#define FS_INPUT_MODE           0
#define FS_OUTPUT_MODE          1
#define FS_APPEND_MODE          2
#define FS_OVERWRITE_MODE       4
#define FS_TEST_INPUT_MODE     10
#define FS_TEST_OUTPUT_MODE    11

#define FS_END_OF_RECORD     "\n"
#define FS_WHITE_SPACES     " \t"
#define FS_END_OF_FIELD       " "
#define FS_RECORD_SIZE      32768
#define FS_BUFFER_SIZE       1024

typedef struct fileDesc_s
{
  FILE           *filePointer;
  char           *fileName;
  int             openMode;
  char           *header;
  struct fileDesc_s *nextFile;
}
fileDesc_t;

typedef fileDesc_t *file_t;

void            NewFile (file_t * f, char *fileName, int openMode, char *header);
void            FreeFile (file_t * f);
void            ResetFile (file_t * f);

#define GetOpenMode( f )  (((f)->openMode==FS_INPUT_MODE)?"r":(((f)->openMode==FS_APPEND_MODE)?"a":"w"))
#define SetOpenMode( f, mode )  ((f)->openMode=mode)
#define GetHeader( f )  (char *)((f)->header)
#define SetHeader( f, str )  ((f)->header = (char *)strdup(str))
#define GetFilePointer( f )  ((f)->filePointer)

#endif

#ifndef __SPOUTNIK_FILE_SET__
#define __SPOUTNIK_FILE_SET__

typedef struct
{
  file_t          firstFile;
  file_t          curFile;
}
fileSetDesc_t;

typedef fileSetDesc_t *fileSet_t;

void            NewFileSet (fileSet_t * fileSet);
void            FreeFileSet (fileSet_t * fileSet);

#define FileSetIsEmpty( fs )  ( (fs)->firstFile==NULL )
void            AddFile (fileSet_t fileSet, char *fileName, int openMode, char *header);
file_t          GetFileByName (fileSet_t fileSet, char *fileName);

#endif

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :     old parameter.h                                           */
/**************************************************************************/
/* Abstract :   type, define and proto for file header processing         */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_PARAMETER__
#define __SPOUTNIK_PARAMETER__

#define StrToLong( str, value )  sscanf( str, "%ld", &(value) );
#define LongToStr( value, str )  sprintf( str, "%ld", value );
#define StrToDouble( str, value )  sscanf( str, "%lf", &(value) );
#define DoubleToStr( value, str )  sprintf( str, "%lf", value );
char           *GetParameter (char *parameter, char *tag, char *str);

#define FreeParameter( str ) if (str) { free(str); (str)=NULL; }

#endif

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :     old function.h                                            */
/**************************************************************************/
/* Abstract :   define usefull function                                   */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_FUNCTION__
#define __SPOUTNIK_FUNCTION__

#define Min(x,y)            ((x)<(y)?(x):(y))
#define Max(x,y)            ((x)>(y)?(x):(y))
#define	Square(x)           ((x)*(x))
#define	Cube(x)             ((x)*(x)*(x))
/* #define ZEROMINUS -1.0e-308
#define ZEROPLUS  1.0e-308 */
#define ZEROPLUS            DBL_MIN
#define SQRT2PI             2.5066282746310002

#endif

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_INDEX__
#define __SPOUTNIK_INDEX__

/*========================================================================*/
/* Class      :    mapindex_t : vector of integer                         */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
typedef struct
{
  long            dim;
  long           *values;
}
indexDesc_t;
typedef indexDesc_t *mapindex_t;

#define GetIndexValue( index, i )  ((index)->values[i])

void            NewIndex (mapindex_t * index, long dim);
void            FreeIndex (mapindex_t * index);
void            CopyIndex (mapindex_t from, mapindex_t to);
void            FlatIndex (mapindex_t * dest, mapindex_t src);
long            Offset (mapindex_t index, mapindex_t size);
mapindex_t      OffsetToIndex (long offset, mapindex_t index, mapindex_t size);

/* a supprimer */
mapindex_t      OffsetToIndex5 (long offset, mapindex_t index, mapindex_t size);
mapindex_t      OffsetToIndex2d (long offset, mapindex_t index, mapindex_t size);
long            IndexRange (mapindex_t index);
mapindex_t      StrToIndex (char *str, mapindex_t index);
char           *IndexToStr (mapindex_t index, char *str);
int             IndexAreEquals (mapindex_t left, mapindex_t right);
mapindex_t      SubstractIndex (mapindex_t res, mapindex_t left, mapindex_t right);
mapindex_t      AddIndex (mapindex_t res, mapindex_t left, mapindex_t right);
double          IndexDistance (mapindex_t left, mapindex_t right);
double          Index2dDistance (mapindex_t left, mapindex_t right);

#endif

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :     old vector.h                                            */
/**************************************************************************/
/* Abstract :   define, type and proto for vector manipulation            */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_VECTOR__
#define __SPOUTNIK_VECTOR__

#define	scalar_t double
#define MAXSCALAR DBL_MAX

/*========================================================================*/
/* Class      :   vector_t                                                */
/* Abstract   : Implementation of real vector type and basic manipulation */
/*                                                                        */
/*========================================================================*/
/* dim = dimension du vecteur = nombre de composantes 
   values = liste des composantes du vecteur
   index = uniquement pour les neurones, sinon null
   pointeur sur un index enregistre dans une liste d'ndex
   au moment de la creation du neurone (NewVector) 
 */
typedef struct
{
  long            dim;
  scalar_t       *values;
  mapindex_t      index;
}
vectorDesc_t;
typedef vectorDesc_t *vector_t;

#define GetVectorValue( vector, i )  ((vector)->values[i])
#define SetVectorValue( vector, i, value )  ((vector)->values[(i)]=(value))

void            NewVector (vector_t * vector, long dim);
void            FreeVector (vector_t * vector);
void            CopyVector (vector_t from, vector_t to);
int             StrToVector (char *str, vector_t vector);
char           *VectorToStr (vector_t vector, char *str);
int             VectorsAreEquals (vector_t left, vector_t right);
void            DoSubstractVectors (scalar_t * pDest, scalar_t * pLeft, scalar_t * pRight,
				    long Size);
vector_t        AddVectors (vector_t res, vector_t left, vector_t right);
scalar_t        xEuclidNorm (vector_t vector);
scalar_t        DoEuclidNorm (scalar_t * Vect, long Size);
scalar_t        EuclidNorm2 (vector_t vector);
scalar_t        AbsNorm (vector_t vector);
scalar_t        AbsNorm2D (vector_t vector);
scalar_t        QuadraticNorm (vector_t vector);
scalar_t        QuadraticDistance (vector_t left, vector_t right);

#define SubstractVectors(res, left, right) DoSubstractVectors(&(res)->values[0],\
                                           &(left)->values[0],&(right)->values[0],(res)->dim)
#define EuclidNorm(vect) DoEuclidNorm(&(vect)->values[0],(vect)->dim)

#endif

/*====================================  ==================================*/
/* Class      : matrix_t                                                  */
/* Abstract   : Implementation of matrix type and basic manipulations     */
/*                                                                        */
/*========================================================================*/
typedef struct
{
  long            dimi;
  long            dimj;
  scalar_t       *values;
}
matrixDesc_t;
typedef matrixDesc_t *matrix_t;

void            NewMatrix (matrix_t * matrix, long dimi, long dimj);
void            FreeMatrix (matrix_t * matrix);

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :     old set.h                                                 */
/**************************************************************************/
/* Abstract :   define, type and proto for data set manipulation          */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_SET__
#define __SPOUTNIK_SET__

#define	elementDesc_t vector_t
#define	element_t vector_t *

typedef struct
{
  element_t       firstElement;
  element_t       curElement;
  long            size;			       /* length of the data set (number of elements) */
  long            index;		       /* position of current element */
  long            dim;			       /* dimension of the elements */
  vector_t        moments;
  vector_t        stat;			       /* vector containing statistic relative to the set */
}
setDesc_t;

typedef setDesc_t *set_t;

#define SET_STATSIZE     4
#define SET_RMS          0
#define SET_BESTRMS      1
#define SET_CONTRAST     2
#define SET_BESTCONTRAST 3

void            NewSet (set_t * set, long size, long dim);
void            FreeSet (set_t * set);
void            LoadSet (set_t * set, file_t setFile);
void            SaveSet (set_t set, file_t setFile);

#define SetIsFree( set )  ((set)==NULL)
#define ForAllElements( set, cursor ) \
for ( (cursor)=(set)->firstElement, (set)->index=0;\
      (set)->index<(set)->size;\
      (cursor)++, ((set)->index)++\
    )
#define ForAllOrderedElements( set, cursor, d ) \
for ( (set)->index=(((set)->size-1)*(d)),\
      (cursor)=(set)->firstElement+(set)->index;\
      (set)->index<(set)->size && (set)->index>=0;\
      (cursor)+=1+(-2*(d)), ((set)->index)+=1+(-2*(d))\
    )
#define	NewElement(element,dim) NewVector(element,dim)
#define FreeElement(element) FreeVector(element)

#endif

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :                                                               */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/
/*
   #ifndef __SPOUTNIK_INDEX__
   #define __SPOUTNIK_INDEX__
 */

/*========================================================================*/
/* Class      :    mapindex_t : vector of integer                            */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
/* typedef struct
   {
   long dim;
   long *values;
   } indexDesc_t;
   typedef indexDesc_t * mapindex_t;

   #define GetIndexValue( index, i )  ((index)->values[i])

   void    NewIndex( mapindex_t *index, long dim );
   void    FreeIndex( mapindex_t *index );
   void    CopyIndex( mapindex_t from, mapindex_t to );
   long    Offset( mapindex_t index, mapindex_t size );
   mapindex_t OffsetToIndex( long offset, mapindex_t index, mapindex_t size );
   mapindex_t OffsetToIndex2d( long offset, mapindex_t index, mapindex_t size );
   long    IndexRange( mapindex_t index );
   mapindex_t StrToIndex( char *str, mapindex_t index );
   char *  IndexToStr( mapindex_t index, char *str ) ;
   int     IndexAreEquals( mapindex_t left, mapindex_t right );
   mapindex_t SubstractIndex( mapindex_t res, mapindex_t left, mapindex_t right );
   mapindex_t AddIndex( mapindex_t res, mapindex_t left, mapindex_t right );
   double       IndexDistance( mapindex_t left, mapindex_t right );
   double       Index2dDistance( mapindex_t left, mapindex_t right );

   #endif

 */

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :    old map.h                                                  */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_POINT__
#define __SPOUTNIK_POINT__

/*========================================================================*/
/* Class      :                                                           */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
#define	pointDesc_t vector_t
#define	point_t vector_t *

#endif

#ifndef __SPOUTNIK_MAP__
#define __SPOUTNIK_MAP__

#define MP_MESH_SEPARATOR "\n\n"
#define MP_ISOLINE_SEPARATOR "\n"

/*========================================================================*/
/* Class      :                                                           */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
typedef struct
{
  point_t         firstPoint;
  point_t         curPoint;
  vector_t        min;
  vector_t        max;
  mapindex_t      size;
  mapindex_t      index;
  long            offset;
  long            range;
}
mapDesc_t;
typedef mapDesc_t *map_t;

#define MapIsFree( map )  ( (map)==NULL )
#define PointByIndex(map,index) ((map)->firstPoint+Offset(index,(map)->size))
#define GetNeighborPoint( map, point, delta )  ((point)+Offset(index,(map)->size))
#define ForAllPoints( map, cursor ) \
    for ( (cursor)=(map)->firstPoint, (map)->offset=0 ;\
          (map)->offset<(map)->range ;\
          ((map)->offset)++, (cursor)++\
        )

void            NewMap (map_t * map, mapindex_t size, vector_t min, vector_t max,
			long pointDim);
void            FreeMap (map_t * map);
void            LoadMap (map_t * map, file_t mapFile);
void            LoadMapIndex (map_t * map, file_t mapFile, long **index);
void            LoadMapLabel (map_t * map, file_t mapFile, vector_t * label);
void            LoadMapProba (map_t * map, file_t mapFile);
void            SaveMap (map_t map, file_t * mapFile);
void            ScaleMap (map_t map);
void            Scale5Map (map_t map, long dim);
scalar_t        K1 (scalar_t u, scalar_t t);
void            SaveLabelMap (map_t map, long *label, file_t * mapFile);
void            SaveIndexMap (map_t map, long *index, file_t * mapFile, double *membr);
void            SaveIndexSaMap (map_t map, long *index, file_t * mapFile, char *topol,
				double *membr);
void            SaveMapOffset (map_t map, file_t mapFile);

#endif

/**************************************************************************/
/* Author :   P. DAIGREMONT (modified by C. AMBROISE )                    */
/* Date   :                                                               */
/* File   :    old som.h                                                  */
/**************************************************************************/
/* Abstract :                                                             */
/**************************************************************************/
/* Parameters :                                                           */
/*                                                                        */
/**************************************************************************/

#ifndef __SPOUTNIK_SOM__
#define __SPOUTNIK_SOM__

#define SomThresholdNeighborhood 0.001

/*========================================================================*/
/* Class      :                                                           */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
typedef struct
{
  map_t           map;
  vector_t        inputVector;
  vector_t        info;
  point_t         winner;
  long            winnerOffset;
  mapindex_t      winnerIndex;
  double          winnerDistance;
  vector_t        distanceVector;
  double          distanceNorm;
  double          neighborhood;
  double          threshold_neighborhood; /* max value to neighborhood */
  double          learningRate;
  double          learningRateMax;
  double          learningRateMin;
  double          smoothDistance;
  double          smoothDistanceMax;
  double          smoothDistanceMin;
  long            cycle;
  long            nbCycle;
  long            freqTest;
  double          step;
  double          stepMax;
  set_t           learnSet;
  set_t           learnSetInfo;
  set_t           testSet;
  set_t           testSetInfo;
  double          stat[4];
  int             tempStochastic;
  int             appBatch;
}
somDesc_t;
typedef somDesc_t *som_t;

#define	SI_SIZE 5
#define	SI_WINNEROFFSET 0
#define	SI_ERROR 1
#define SI_DENSITY 2
#define SI_WINNERINDEX 4
#define SI_REGRESSION 2
#define SI_REGRESSION_ERROR 3
#define	SetInfo(set,offset,i,val) (*((set)->firstElement+(offset)))->values[i]=(double)(val)
#define	GetInfo(set,offset,i,val) (*((set)->firstElement+(offset)))->values[i]

/*--- Lexique des projets */
#define PRJ_LEARNFILE "learnFile="
#define PRJ_VALIDFILE "validFile="
#define PRJ_RESFILE "resFile="
#define PRJ_NETFILE "netFile="
#define PRJ_NETSIZE "netSize="
#define PRJ_NETINPUTDIM "netInputDim="
#define PRJ_MAPMIN "mapMin="
#define PRJ_MAPMAX "mapMax="
#define PRJ_NETLEARNMASK "netLearnMask="
#define PRJ_NETINITIALLR "netInitialLR="
#define PRJ_NETFINALLR "netFinalLR="
#define PRJ_NETNBCYCLE "netNbCycle="
#define PRJ_NETCYCLE "netCycle="
#define PRJ_STATRATE "statRate="

scalar_t        Neighborhood (mapindex_t w, mapindex_t k, scalar_t s0, scalar_t beta);
scalar_t        K (scalar_t u, scalar_t t);
void            ClassSom (som_t som);
void            LearnSom (som_t som);
void            LearnAdaptSom (som_t som);
void            NewSom (som_t * som, mapindex_t mapSize, vector_t mapMin, vector_t mapMax,
			long inputDim);
void            FreeSom (som_t * som);
void            LoadSom (som_t * som, file_t file);
void            CycleSom (som_t som);
void            SomStat (som_t som);
void            InitSom (som_t * som, set_t set);
scalar_t        Rms (set_t set, int errorIndex);
scalar_t        Contrast (set_t set, map_t map);

#define ForAllCycles( som )\
for ( (som)->step=(som)->cycle, \
      (som)->stepMax=(double)(som)->nbCycle*(double)(som)->learnSet->size ; \
      (som)->cycle<(som)->nbCycle ; \
      (som)->cycle++ \
    )
#endif

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

#ifndef __SPOUTNIK_PSOM__
#define __SPOUTNIK_PSOM__

#define RBF_TEMPERATURE 2
#define SIGMA_MULT 2
/*========================================================================*/
/* Class      :                                                           */
/* Abstract   :                                                           */
/*                                                                        */
/*========================================================================*/
typedef struct
{
  som_t           som;
  map_t           dev;
  map_t           activation;
  matrix_t        mapdistance;		       /* Matrice des Distances sur la carte */
  int             StochasticStep;	       /* ON ou OFF pour specifier l'existence d'une etape stochastique */
  int             ComputeVariance;	       /* ON ou OFF pour specifier si les variances sont calculees */
  int             ComputeMoyenne;	       /* ON ou OFF pour specifier si les moyennes  sont calculees */
  int             VarieSmoothD;		       /* ON ou OFF pour specifier si les moyennes  sont calculees */
}
psomDesc_t;
typedef psomDesc_t *psom_t;

scalar_t        Kp (scalar_t u, scalar_t t);
scalar_t        Kp2 (scalar_t u, scalar_t t);
scalar_t        Rbf1 (vector_t z, vector_t w, scalar_t sigma);
scalar_t        Rbf (scalar_t d, scalar_t sigma, scalar_t dim);
scalar_t        Bell (vector_t z, vector_t w, scalar_t sigma);
void            ClassPsom (psom_t psom);
void            InitPsom (psom_t * psom, set_t set);
void            NewPsom (psom_t * psom, mapindex_t mapSize, vector_t mapMin, vector_t mapMax,
			 long inputDim);
void            FreePsom (psom_t * psom);
void            LoadPsom (psom_t * psom, file_t mapFile, file_t devFile);
void            Load2Psom (psom_t * psom, file_t mapFile, file_t devFile);
void            CyclePsom (psom_t psom);
void            PsomStat (psom_t psom);
void            SaveActivation (psom_t psom, file_t resFile);
void            SaveProbability (psom_t psom, file_t resFile);

#endif
