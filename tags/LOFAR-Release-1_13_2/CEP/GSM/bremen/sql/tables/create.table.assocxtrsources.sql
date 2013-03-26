--drop table assocxtrsources;
/**
 * This table stores the information about the sources that
 * could be associated.
 * src_type:        Either 'X' or 'C', for associations
 *                  in extractedsources or catalogedsources
 * xrtsrc_id:       This is the xtrsrcid that corresponds to the
 *                  first detection
 * assoc_xtrsrcid:  This is the id of the source that could be
 *                  associated to a previously detection
 *                  (corresponding to assoc_xtrsrcid)
 */
--CREATE SEQUENCE "seq_assocxtrsources" AS INTEGER;

CREATE TABLE assocxtrsources
  /*(id INT DEFAULT NEXT VALUE FOR "seq_assocxtrsources"*/
  (xtrsrc_id INT NOT NULL
  ,runcat_id INT NOT NULL
  ,weight double NULL default 1
  ,distance_arcsec double NULL
  ,lr_method INT NULL DEFAULT 0
  ,r double NULL
  ,lr double NULL
  /*,PRIMARY KEY (id)
  ,FOREIGN KEY (xtrsrc_id) REFERENCES extractedsources (xtrsrcid)
  ,FOREIGN KEY (assoc_xtrsrc_id) REFERENCES extractedsources (xtrsrcid)*/
  )
;

