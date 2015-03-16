/*+-------------------------------------------------------------------+
 *| This script loads the external WENSS catalogue into the DB        |
 *| All WENSS data were subtracted from the csv file from the vizier  |
 *| website.The wenss-header.txt gives a description of the columns   |
 *| in the csv file.                                                  |
 *| The WENSS catalogue consists of two surveys:                      |
 *| (1) the main part, at 325 MHz, which contains the sources between |
 *|     28 < decl < 76                                                |
 *| (2) the polar part, at 352 MHz, which contains the sources with   |
 *|     decl > 72.                                                    |
 *| Therefore, we will create two separate WENSS catalogues, a main   |
 *| and polar part.                                                   |
 *+-------------------------------------------------------------------+
 *| Bart Scheers                                                      |
 *| 2011-02-16                                                        |
 *+-------------------------------------------------------------------+
 *| Open Questions/TODOs:                                             |
 *| (1) If we dump the default wenss data into a file, we can use that|
 *}     for even faster load.                                         |
 *+-------------------------------------------------------------------+
 */
DECLARE icatid_main, icatid_pole INT;
DECLARE ifreq_eff_main, ifreq_eff_pole DOUBLE;
DECLARE iband_main, iband_pole INT;

/*see Rengelink et al.(1997) Eq.9*/
DECLARE C1_sq, C2_sq DOUBLE;
SET C1_sq = 0.0016;
SET C2_sq = 1.69;

SET icatid_main = 5;
SET icatid_pole = 6;

INSERT INTO catalogs
  (catid
  ,catname
  ,fullname
  ) VALUES 
  (icatid_main
  ,'WENSSm'
  ,'WEsterbork Nortern Sky Survey, Main Catalogue @ 325 MHz'
  )
  ,
  (icatid_pole
  ,'WENSSp'
  ,'WEsterbork Nortern Sky Survey, Polar Catalogue @ 352 MHz'
  )
;

SET ifreq_eff_main = 325000000.0;
/*SET iband_main = getBand(ifreq_eff_main, 10000000.0);*/
SET iband_main = getBand(ifreq_eff_main);
SET ifreq_eff_pole = 352000000.0;
/*SET iband_pole = getBand(ifreq_eff_pole, 10000000.0);*/
SET iband_pole = getBand(ifreq_eff_pole);


/*DROP TABLE aux_catalogedsources;*/

CREATE TABLE aux_catalogedsources
  (aviz_RAJ2000 DOUBLE
  ,aviz_DEJ2000 DOUBLE
  ,aorig_catsrcid INT
  ,aname VARCHAR(16)
  ,af_name VARCHAR(8)
  ,awsrt_RAB1950 VARCHAR(12)
  ,awsrt_DEB1950 VARCHAR(12)
  ,adummy1 VARCHAR(20)
  ,adummy2 VARCHAR(20)
  ,aflg1 VARCHAR(2)
  ,aflg2 VARCHAR(1)
  ,a_I DOUBLE
  ,a_S DOUBLE
  ,amajor DOUBLE
  ,aminor DOUBLE
  ,aPA DOUBLE
  ,arms DOUBLE
  ,aframe VARCHAR(20)
  )
;

COPY 229420 RECORDS 
INTO aux_catalogedsources 
FROM
/* Uncomment here the absolute server path to the csv catalogue file  
 '/something/like/catfiles/vlss/VLSS-all_strip.csv' */
USING DELIMITERS ';', '\n' 
;

INSERT INTO catalogedsources
  (orig_catsrcid
  ,catsrcname
  ,cat_id
  ,band
  ,ra
  ,decl
  ,zone
  ,ra_err
  ,decl_err
  ,freq_eff
  ,x
  ,y
  ,z
  ,src_type
  ,fit_probl
  ,pa
  ,major
  ,minor
  ,i_peak_avg
  ,i_peak_avg_err
  ,i_int_avg
  ,i_int_avg_err
  ,frame
  )
  SELECT aorig_catsrcid
        ,CONCAT(TRIM(aname), af_name)
        ,CASE WHEN aframe LIKE 'WNH%'
              THEN icatid_main
              ELSE icatid_pole
         END
        ,CASE WHEN aframe LIKE 'WNH%'
              THEN iband_main
              ELSE iband_pole
         END
        ,aviz_RAJ2000
        ,aviz_DEJ2000
        ,CAST(FLOOR(aviz_DEJ2000) AS INTEGER)
        ,CASE WHEN a_I / arms >= 10 
              THEN 1.5
              ELSE CASE WHEN amajor <> 0
                        THEN SQRT(2.25 + arms * arms * (amajor * amajor * SIN(RADIANS(apa)) * SIN(RADIANS(apa)) 
                                                       + aminor * aminor * COS(RADIANS(apa)) * COS(RADIANS(apa))
                                                       ) / (1.69 * a_I * a_I))
                        ELSE SQRT(2.25 + arms * arms * 2916 / (1.69 * a_I * a_I))
                   END
         END
        ,CASE WHEN a_I / arms >= 10 
              THEN 1.5
              ELSE CASE WHEN amajor <> 0
                        THEN SQRT(2.25 + arms * arms * (amajor * amajor * COS(RADIANS(apa)) * COS(RADIANS(apa)) 
                                                       + aminor * aminor * SIN(RADIANS(apa)) * SIN(RADIANS(apa))
                                                       ) / (1.69 * a_I * a_I)) 
                        ELSE SQRT(2.25 + arms * arms * 2916 
                                 / (1.69 * a_I * a_I * SIN(RADIANS(aviz_DEJ2000)) * SIN(RADIANS(aviz_DEJ2000)))
                                 )
                   END
         END
        ,CASE WHEN aframe LIKE 'WNH%'
              THEN ifreq_eff_main
              ELSE ifreq_eff_pole
         END
        ,COS(RADIANS(aviz_DEJ2000)) * COS(RADIANS(aviz_RAJ2000))
        ,COS(RADIANS(aviz_DEJ2000)) * SIN(RADIANS(aviz_RAJ2000))
        ,SIN(RADIANS(aviz_DEJ2000))
        ,aflg1
        ,CASE WHEN aflg2 = '*'
              THEN aflg2
              ELSE NULL
         END
        ,apa
        ,amajor
        ,aminor
        ,a_I / 1000
        ,SQRT(C1_sq + C2_sq * (arms / a_I) * (arms / a_I)) * a_I / 1000
        ,a_S / 1000
        ,SQRT(C1_sq + C2_sq * (arms / a_S) * (arms / a_S)) * a_S / 1000
        ,aframe
    FROM aux_catalogedsources
   WHERE a_S > 0
--     AND af_name a_S > 0
;

DROP TABLE aux_catalogedsources;

