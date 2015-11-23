-- resourceassignment password for testing on mcu005 is the same as the password on the president's luggage +6
-- psql resourceassignment -U resourceassignment -f fill_database.sql -W
BEGIN;
INSERT INTO virtual_instrument.unit VALUES (0, 'station'),(1, 'bytes');
INSERT INTO virtual_instrument.resource_type VALUES (0, 'station', 0),(1, 'storage', 1);
INSERT INTO virtual_instrument.resource VALUES (0, 'CS001', 0), (1,'CS002', 0), (2, 'cep4_storage', 1);
INSERT INTO virtual_instrument.resource_group_type VALUES (0, 'stations'),(1, 'cluster');
INSERT INTO virtual_instrument.resource_group VALUES (0, 'CORE', 0),(1, 'CEP4', 1);
INSERT INTO virtual_instrument.resource_to_resource_group VALUES (0, 0, 0),(1, 1, 0), (2, 2, 1);
INSERT INTO virtual_instrument.resource_group_to_resource_group VALUES (0, 0, NULL),(1, 1, NULL);
INSERT INTO resource_allocation.task_status VALUES (0, 'SCHEDULED'),(1, 'CONFLICT');
INSERT INTO resource_allocation.task_type VALUES (0, 'OBSERVATION'),(1, 'PIPELINE');
INSERT INTO resource_allocation.specification VALUES (0, '2015-11-05 12:00:00', '2015-11-05 12:30:00', 'key=value'),
(1, '2015-11-05 13:00:00', '2015-11-05 14:00:00', 'key=1');
INSERT INTO resource_allocation.task VALUES (0, 654321, 12345, 0, 0, 0),(1, 765432, 2654321, 1, 1, 1);
INSERT INTO resource_allocation.claim_session VALUES (0, 'renting', 0, '2015-11-05 12:00:00', 'renting-token'),
(1, 'paulus', 1, '2015-11-05 14:00:00', 'paulus-token');
INSERT INTO resource_allocation.resource_claim_status VALUES (0, 'CLAIMED'),(1, 'ALLOCATED');
INSERT INTO resource_allocation.resource_claim VALUES (0, 0, 0, '2015-11-05 12:00:00', '2015-11-05 12:30:00', 0, 0, 1),
(1, 2, 1, '2015-11-05 13:00:00', '2015-11-05 14:00:00', 1, 1, 1234);
INSERT INTO resource_monitoring.resource_capacity VALUES (0, 0, 1, 1), (1, 1, 1,1 ), (2, 2, 15000, 100000);
INSERT INTO resource_monitoring.resource_availability VALUES (0, 0, TRUE), (1, 1, TRUE), (2, 2, TRUE);
INSERT INTO resource_monitoring.resource_group_availability VALUES (0, 0, TRUE), (1, 1, TRUE);
INSERT INTO resource_allocation.config VALUES (0, 'max_fill_percentage_cep4', '85.00'), (1, 'claim_timeout', '172800');
COMMIT;
