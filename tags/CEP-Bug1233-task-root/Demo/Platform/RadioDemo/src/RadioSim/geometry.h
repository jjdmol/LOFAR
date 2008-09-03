#ifndef Geometry_h
#define Geometry_h


/*    Position of stations in meters. */
/*    first coordinate is x-pos (>0 = north), second y-pos(>0 = east). */

  
/*                 (1)                */
/*                  |                 */
/*                  |                 */
/*                 (0)                */
/*                /   \               */
/*               /     \              */
/*             (2)     (3)            */
   

/* Station 0 is situated in orgin */
/* All angles are 120 degrees. */
/* Distance between stations is 2 km. */

/* static */ const float STAT_0_POS[2] = {    0.0,    0.0};
/* static */ const float STAT_1_POS[2] = {    0.0, 2000.0};
/* static */ const float STAT_2_POS[2] = {-1732.1,-1000.0};
/* static */ const float STAT_3_POS[2] = { 1732.1,-1000.0};

/* static */ const float* STAT_POS[STATIONS] = {STAT_0_POS,
			           STAT_1_POS,
			           STAT_2_POS,
			           STAT_3_POS};


/*  Position of antennas in meters */
/*  first coordinate is x-pos, second y-pos. */

/*            (0)-------(1)                */
/*             |         |                 */
/*             |         |                 */
/*             |   (*)   |                 */
/*             |         |                 */
/*             |         |                 */
/*            (2)-------(3)                */

/*  (*) is orgin */
/*  Distance between antennas is 5 m. */

const /* static */ float ANT_0_POS[2] = {-2.5, 2.5};
const /* static */ float ANT_1_POS[2] = { 2.5, 2.5};
const /* static */ float ANT_2_POS[2] = {-2.5,-2.5};  
const /* static */ float ANT_3_POS[2] = { 2.5,-2.5};

const /* static */ float* ANT_POS[ELEMENTS] = { ANT_0_POS,
					  ANT_1_POS,
					  ANT_2_POS,
					  ANT_3_POS};



#endif
