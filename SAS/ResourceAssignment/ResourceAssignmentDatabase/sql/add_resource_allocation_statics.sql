-- resourceassignment password for testing on mcu005 is the same as the password on the president's luggage +6
-- psql resourceassignment -U resourceassignment -f add_resource_allocation_statics.sql -W
BEGIN;

INSERT INTO resource_allocation.task_status VALUES (200, 'prepared'), (300, 'approved'), (320, 'on_hold'), (335, 'conflict'),
(350, 'prescheduled'), (400, 'scheduled'), (500, 'queued'), (600, 'active'), (900, 'completing'), (1000, 'finished'), (1100, 'aborted'),
(1150, 'error'), (1200, 'obsolete'); -- This is the list from OTDB, we'll need to merge it with the list from MoM in the future, might use different indexes?
INSERT INTO resource_allocation.task_type VALUES (0, 'observation'),(1, 'pipeline'); -- We'll need more types
INSERT INTO resource_allocation.resource_claim_status VALUES (0, 'claimed'), (1, 'allocated'), (2, 'conflict');
INSERT INTO resource_allocation.resource_claim_property_type VALUES (0, 'nr_of_is_files'),(1, 'nr_of_cs_files'),(2, 'nr_of_uv_files'),(3, 'nr_of_im_files'),(4, 'nr_of_img_files'),(5, 'nr_of_pulp_files'),(6, 'nr_of_cs_stokes'),(7, 'nr_of_is_stokes'),(8, 'is_file_size'),(9, 'cs_file_size'),(10, 'uv_file_size'),(11, 'im_file_size'),(12, 'img_file_size'),(13, 'nr_of_pulp_files'),(14, 'nr_of_tabs');
INSERT INTO resource_allocation.config VALUES (0, 'max_fill_percentage_cep4', '85.00'), (1, 'claim_timeout', '172800'); -- Just some values 172800 is two days in seconds
INSERT INTO resource_allocation.conflict_reason
VALUES
(1, 'Not enough total free storage space'),
(2, 'Storage node inactive'),
(3, 'Number of storage nodes available less than minimum required'),
(4, 'No suitable storage options found'),
(5, 'No storage nodes available'),
(6, 'Not enough available storage nodes for required bandwidth'),
(7, 'Network bandwidth to storage node too high'),
(8, 'Bandwidth required for single file too high');
COMMIT;
