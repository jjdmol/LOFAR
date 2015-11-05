CREATE INDEX i_assoc_extracted ON assocxtrsources USING btree (xtrsrc_id );
CREATE INDEX i_assoc_runcat    ON assocxtrsources USING btree (runcat_id );


CREATE INDEX i_extract_xyz ON extractedsources USING btree (x , y , z );
CREATE INDEX i_image       ON extractedsources USING btree (image_id );
CREATE INDEX i_xtrsrc      ON extractedsources USING btree (xtrsrcid );


CREATE INDEX i_runcat_first ON runningcatalog USING btree (first_xtrsrc_id );
CREATE INDEX i_runcat_xyz   ON runningcatalog USING btree (x , y , z );
CREATE INDEX i_runcat_id    ON runningcatalog USING btree (runcatid, band, stokes);
CREATE INDEX i_runcat_parent ON runningcatalog USING btree (parent_runcat_id, band, stokes);
CREATE INDEX i_runcat_parent0 ON runningcatalog USING btree (parent_runcat_id)



