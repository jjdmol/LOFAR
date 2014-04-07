/**
 * This table contains the images that are being processed.
 * The only format for now is FITS. The HDF5 format will be implemented
 * later.
 * An image is characterised by
 *      - integration time (tau)
 *      - frequency band (band) 
 *      - timestamp (seq_nr).
 * A group of images that belong together (not specified any further)
 * are in the same data set (they have the same ds_id).
 * tau_time in seconds (ref. tau)
 * freq_eff in Hz (ref. band)
 * taustart_timestamp in YYYY-MM-DD-HH:mm:ss:nnnnnn, but without 
 *                    interpunctions (ref. seq_nr)
 * bmaj, bmin, bpa are the major, minor axes of the synthesized beam in degrees
 *                 NOTE that it are NOT the semimajor axes.
 * centr_ra and _decl are the central coordinates (J2000) of the image in degrees.
 */
CREATE SEQUENCE "seq_images" AS INTEGER;

CREATE TABLE images 
  (imageid INT DEFAULT NEXT VALUE FOR "seq_images"
  ,ds_id INT NOT NULL
  ,tau INT NOT NULL
  ,band INT NOT NULL
  ,stokes CHAR(1) NOT NULL DEFAULT 'I'
  ,tau_time DOUBLE NOT NULL
  ,freq_eff DOUBLE NOT NULL
  ,freq_bw DOUBLE NULL
  ,taustart_ts TIMESTAMP NOT NULL
  ,bmaj DOUBLE NOT NULL
  ,bmin DOUBLE NOT NULL
  ,bpa DOUBLE NOT NULL
  ,centr_ra DOUBLE NOT NULL
  ,centr_decl DOUBLE NOT NULL
  ,x DOUBLE NOT NULL
  ,y DOUBLE NOT NULL
  ,z DOUBLE NOT NULL
  ,url VARCHAR(120) NULL
  ,PRIMARY KEY (imageid)
  ,FOREIGN KEY (ds_id) REFERENCES datasets (dsid)
  ,FOREIGN KEY (band) REFERENCES frequencybands (freqbandid)
  )
;

