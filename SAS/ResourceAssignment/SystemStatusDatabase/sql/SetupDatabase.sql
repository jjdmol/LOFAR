-- DROP DATABASE IF EXISTS datamanager;
-- CREATE DATABASE datamanager
--   WITH OWNER = peterzon
 --      ENCODING = 'UTF8'
--       TABLESPACE = pg_default
--       LC_COLLATE = 'en_US.UTF-8'
--       LC_CTYPE = 'en_US.UTF-8'
--       CONNECTION LIMIT = -1;
--CREATE SCHEMA datamanager;
--SET SCHEMA 'datamanager';

DROP TABLE dataproducts; 
DROP TABLE datapaths;
DROP TABLE IF EXISTS hosts;

CREATE TABLE datapaths (
  id bigint  NOT NULL,
  hostid bigint  NOT NULL,
  path varchar(255) NOT NULL,
  totalspace bigint  NOT NULL,
  usedspace bigint  NOT NULL,
  claimedspace bigint  NOT NULL,
  PRIMARY KEY  (id)
)WITH (
  OIDS=FALSE
);

CREATE TABLE dataproducts (
  id serial NOT NULL ,
  name varchar(255) NOT NULL,
  projectid bigint  default NULL,
  size bigint  default NULL,
  status bigint  default NULL,
  datapathid bigint  default NULL,
  PRIMARY KEY  (id)
)WITH (
  OIDS=FALSE
);

ALTER SEQUENCE dataproducts_id_seq RESTART WITH 323351;


DROP TABLE IF EXISTS gatherer;
CREATE TABLE gatherer (
  timestamp bigint  default NULL
)WITH (
  OIDS=FALSE
);
INSERT INTO gatherer VALUES (0);

CREATE TABLE hosts (
  id bigint  NOT NULL,
  hostname varchar(255) NOT NULL,
  groupid bigint  NOT NULL,
  statusid bigint  NOT NULL,
  PRIMARY KEY  (id)
)WITH (
  OIDS=FALSE
);

DROP TABLE IF EXISTS projects;
CREATE TABLE projects (
  id serial,
  name varchar(255) NOT NULL,
  PRIMARY KEY  (id)
)WITH (
  OIDS=FALSE
);

DROP TABLE IF EXISTS servergroups;
DROP TYPE IF EXISTS yesno;
CREATE TYPE yesno AS ENUM('y','n');

CREATE TABLE servergroups (
  id bigint  NOT NULL,
  groupname varchar(20) NOT NULL,
  active yesno default NULL,
  PRIMARY KEY  (id)
)WITH (
  OIDS=FALSE
);
INSERT INTO servergroups VALUES (0,'storagenodes','y'),
    (1,'computenodes','y'),
    (2,'archivenodes','y'),
    (3,'locusnodes','y'),
    (4,'cep4','y');

DROP TABLE IF EXISTS states;
CREATE TABLE states (
  id bigint  NOT NULL,
  statename varchar(255) NOT NULL,
  PRIMARY KEY  (id)
)WITH (
  OIDS=FALSE
);
INSERT INTO states VALUES (0,'Inactive'),
    (1,'Active');

DROP TABLE IF EXISTS export_jobs;
DROP TYPE IF EXISTS status_type;

CREATE TYPE status_type AS ENUM('unknown','scheduled','running','done');

CREATE TABLE export_jobs (
  id bigint  NOT NULL,
  description varchar(255) NOT NULL,
  start varchar(25) NOT NULL,
  update varchar(25) NOT NULL,
  username varchar(25) NOT NULL,
  state varchar(25) NOT NULL,
  name varchar(50) NOT NULL,
  project varchar(50) NOT NULL,
  location varchar(25) NOT NULL,
  status status_type default 'unknown' NOT NULL,
  PRIMARY KEY  (id)
)WITH (
  OIDS=FALSE
);

DROP TABLE IF EXISTS observations;
CREATE TABLE observations (
  obsid varchar(40) NOT NULL,
  job_id bigint  NOT NULL,
  nr_files bigint  NOT NULL
)WITH (
  OIDS=FALSE
);

DROP TABLE IF EXISTS archiving_queues;
CREATE TABLE archiving_queues (
  name varchar(20) NOT NULL,
  length bigint  NOT NULL
)WITH (
  OIDS=FALSE
);

INSERT INTO archiving_queues VALUES('main',0);;

INSERT INTO datapaths VALUES (1,1,'/data1',0,0,0),
    (2,1,'/data2',0,0,0),
    (3,1,'/data3',0,0,0),
    (4,1,'/data4',0,0,0),
    (5,2,'/data1',0,0,0),
    (6,2,'/data2',0,0,0),
    (7,2,'/data3',0,0,0),
    (8,2,'/data4',0,0,0),
    (9,3,'/data1',0,0,0),
    (10,3,'/data2',0,0,0),
    (11,3,'/data3',0,0,0),
    (12,3,'/data4',0,0,0),
    (13,4,'/data1',0,0,0),
    (14,4,'/data2',0,0,0),
    (15,4,'/data3',0,0,0),
    (16,4,'/data4',0,0,0),
    (17,5,'/data1',0,0,0),
    (18,5,'/data2',0,0,0),
    (19,5,'/data3',0,0,0),
    (20,5,'/data4',0,0,0),
    (21,6,'/data1',0,0,0),
    (22,6,'/data2',0,0,0),
    (23,6,'/data3',0,0,0),
    (24,6,'/data4',0,0,0),
    (25,7,'/data1',0,0,0),
    (26,7,'/data2',0,0,0),
    (27,7,'/data3',0,0,0),
    (28,7,'/data4',0,0,0),
    (29,8,'/data1',0,0,0),
    (30,8,'/data2',0,0,0),
    (31,8,'/data3',0,0,0),
    (32,8,'/data4',0,0,0),
    (33,9,'/data1',0,0,0),
    (34,9,'/data2',0,0,0),
    (35,9,'/data3',0,0,0),
    (36,9,'/data4',0,0,0),
    (37,10,'/data1',0,0,0),
    (38,10,'/data2',0,0,0),
    (39,10,'/data3',0,0,0),
    (40,10,'/data4',0,0,0),
    (41,11,'/data1',0,0,0),
    (42,11,'/data2',0,0,0),
    (43,11,'/data3',0,0,0),
    (44,11,'/data4',0,0,0),
    (45,12,'/data1',0,0,0),
    (46,12,'/data2',0,0,0),
    (47,12,'/data3',0,0,0),
    (48,12,'/data4',1,1,0),
    (49,13,'/data1',0,0,0),
    (50,13,'/data2',0,0,0),
    (51,13,'/data3',0,0,0),
    (52,13,'/data4',0,0,0),
    (53,14,'/data1',0,0,0),
    (54,14,'/data2',0,0,0),
    (55,14,'/data3',0,0,0),
    (56,14,'/data4',1,1,0),
    (57,15,'/data1',0,0,0),
    (58,15,'/data2',0,0,0),
    (59,15,'/data3',0,0,0),
    (60,15,'/data4',0,0,0),
    (61,16,'/data1',0,0,0),
    (62,16,'/data2',0,0,0),
    (63,16,'/data3',0,0,0),
    (64,16,'/data4',1,1,0),
    (65,17,'/data1',0,0,0),
    (66,17,'/data2',0,0,0),
    (67,17,'/data3',0,0,0),
    (68,17,'/data4',1,1,0),
    (69,18,'/data1',0,0,0),
    (70,18,'/data2',0,0,0),
    (71,18,'/data3',0,0,0),
    (72,18,'/data4',1,1,0),
    (73,19,'/data1',0,0,0),
    (74,19,'/data2',0,0,0),
    (75,19,'/data3',0,0,0),
    (76,19,'/data4',1,1,0),
    (77,20,'/data1',0,0,0),
    (78,20,'/data2',0,0,0),
    (79,20,'/data3',0,0,0),
    (80,20,'/data4',1,1,0),
    (81,21,'/data1',0,0,0),
    (82,21,'/data2',0,0,0),
    (83,21,'/data3',0,0,0),
    (84,21,'/data4',1,1,0),
    (85,22,'/data1',0,0,0),
    (86,22,'/data2',0,0,0),
    (87,22,'/data3',0,0,0),
    (88,22,'/data4',1,1,0),
    (89,23,'/data1',0,0,0),
    (90,23,'/data2',0,0,0),
    (91,23,'/data3',0,0,0),
    (92,23,'/data4',1,1,0),
    (93,24,'/data1',0,0,0),
    (94,24,'/data2',0,0,0),
    (95,24,'/data3',0,0,0),
    (96,24,'/data4',1,1,0),
    (97,25,'/data',0,0,0),
    (98,26,'/data',0,0,0),
    (99,27,'/data',0,0,0),
    (100,28,'/data',0,0,0),
    (101,29,'/data',0,0,0),
    (102,30,'/data',0,0,0),
    (103,31,'/data',0,0,0),
    (104,32,'/data',1,1,0),
    (105,33,'/data',1,1,0),
    (106,34,'/data',1,1,0),
    (107,35,'/data',1,1,0),
    (108,36,'/data',1,1,0),
    (109,37,'/data',1,1,0),
    (110,38,'/data',1,1,0),
    (111,39,'/data',1,1,0),
    (112,40,'/data',1,1,0),
    (113,41,'/data',1,1,0),
    (114,42,'/data',1,1,0),
    (115,43,'/data',1,1,0),
    (116,44,'/data',1,1,0),
    (117,45,'/data',1,0,0),
    (118,46,'/data',1,1,0),
    (119,47,'/data',1,1,0),
    (120,48,'/data',1,1,0),
    (121,49,'/data',1,1,0),
    (122,50,'/data',1,1,0),
    (123,51,'/data',1,1,0),
    (124,52,'/data',1,1,0),
    (125,53,'/data',1,1,0),
    (126,54,'/data',1,1,0),
    (127,55,'/data',1,1,0),
    (128,56,'/data',1,1,0),
    (129,57,'/data',1,1,0),
    (130,58,'/data',1,1,0),
    (131,59,'/data',1,1,0),
    (132,60,'/data',1,1,0),
    (133,61,'/data',1,1,0),
    (134,62,'/data',1,1,0),
    (135,63,'/data',1,1,0),
    (136,64,'/data',1,1,0),
    (137,65,'/data',1,1,0),
    (138,66,'/data',1,1,0),
    (139,67,'/data',1,1,0),
    (140,68,'/data',1,1,0),
    (141,69,'/data',1,1,0),
    (142,70,'/data',1,1,0),
    (143,71,'/data',1,1,0),
    (144,72,'/data',1,1,0),
    (145,73,'/data',1,1,0),
    (146,74,'/data',1,1,0),
    (147,75,'/data',1,1,0),
    (148,76,'/data',1,1,0),
    (149,77,'/data',1,1,0),
    (150,78,'/data',1,1,0),
    (151,79,'/data',1,1,0),
    (152,80,'/data',1,1,0),
    (153,81,'/data',1,1,0),
    (154,82,'/data',1,1,0),
    (155,83,'/data',1,1,0),
    (156,84,'/data',1,1,0),
    (157,85,'/data',1,1,0),
    (158,86,'/data',1,1,0),
    (159,87,'/data',1,1,0),
    (160,88,'/data',1,1,0),
    (161,89,'/data',1,1,0),
    (162,90,'/data',1,1,0),
    (163,91,'/data',1,1,0),
    (164,92,'/data',1,1,0),
    (165,93,'/data',1,1,0),
    (166,94,'/data',1,1,0),
    (167,95,'/data',1,1,0),
    (168,96,'/data',1,1,0),
    (169,97,'/data',1,1,0),
    (170,98,'/data',1,1,0),
    (171,99,'/data',1,1,0),
    (172,100,'/data',1,1,0),
    (173,101,'/data',1,1,0),
    (174,102,'/data',1,1,0),
    (175,103,'/data',1,1,0),
    (176,104,'/data',1,1,0),
    (177,105,'/data',1,1,0),
    (178,106,'/data',1,1,0),
    (179,107,'/data',1,1,0),
    (180,108,'/data',1,1,0),
    (181,109,'/data',1,1,0),
    (182,110,'/data',1,1,0),
    (183,111,'/data',1,1,0),
    (184,112,'/data',1,1,0),
    (185,113,'/data',1,1,0),
    (186,114,'/data',1,1,0),
    (187,115,'/data',1,1,0),
    (188,116,'/data',1,1,0),
    (189,117,'/data',1,1,0),
    (190,118,'/data',1,1,0),
    (191,119,'/data',1,1,0),
    (192,120,'/data',1,1,0),
    (193,121,'/data',1,1,0),
    (194,122,'/data',1,1,0),
    (195,123,'/data',1,1,0),
    (196,124,'/data',1,1,0),
    (197,125,'/data',1,1,0),
    (198,126,'/data',1,1,0),
    (199,127,'/data',1,1,0),
    (200,128,'/data',1,1,0),
    (201,129,'/data',1,1,0),
    (202,130,'/data',1,1,0),
    (203,131,'/data',1,1,0),
    (204,132,'/data',1,1,0),
    (205,133,'/data',1,1,0),
    (206,134,'/data',1,1,0),
    (207,135,'/data',1,1,0),
    (208,136,'/data',1,1,0),
    (209,137,'/data',1,1,0),
    (210,138,'/data',1,1,0),
    (211,139,'/data',1,1,0),
    (212,140,'/data',1,1,0),
    (213,141,'/data',1,1,0),
    (214,142,'/data',1,1,0),
    (215,143,'/data',1,1,0),
    (216,144,'/data',1,1,0),
    (217,145,'/data',1,1,0),
    (218,146,'/data',1,1,0),
    (219,147,'/data',1,1,0),
    (220,148,'/data',1,1,0),
    (221,149,'/data',1,1,0),
    (222,150,'/data',1,1,0),
    (223,151,'/data',1,1,0),
    (224,152,'/data',1,1,0),
    (225,153,'/data',1,1,0),
    (226,154,'/data',1,1,0),
    (227,155,'/data',1,1,0),
    (228,156,'/data',1,1,0),
    (229,157,'/data',1,1,0),
    (230,158,'/data',1,1,0),
    (231,159,'/data',1,1,0),
    (232,160,'/data',1,1,0),
    (233,161,'/data',1,1,0),
    (234,162,'/data',1,1,0),
    (235,163,'/data',1,1,0),
    (236,164,'/data',1,1,0),
    (237,165,'/data',1,1,0),
    (238,166,'/data',1,1,0),
    (239,167,'/data',1,1,0),
    (240,168,'/data',1,1,0),
    (241,169,'/data',1,1,0),
    (242,170,'/data',1,1,0),
    (243,171,'/data',1,1,0),
    (244,172,'/data',1,1,0),
    (245,173,'/data',1,1,0),
    (246,174,'/data',1,1,0),
    (247,175,'/data',1,1,0),
    (248,176,'/data',1,1,0),
    (249,177,'/data',1,1,0),
    (250,178,'/data',1,1,0),
    (251,179,'/data',1,1,0),
    (252,180,'/data',1,1,0),
    (253,181,'/data',1,1,0),
    (254,182,'/data',1,1,0),
    (255,183,'/data',1,1,0),
    (256,184,'/data',1,1,0),
    (257,185,'/data',1,1,0),
    (258,186,'/data',1,1,0),
    (259,187,'/data',1,1,0),
    (260,188,'/data',1,1,0),
    (261,189,'/data',1,1,0),
    (262,190,'/data',1,1,0),
    (263,191,'/data',1,1,0),
    (264,192,'/data',1,1,0),
    (265,193,'/data',1,1,0),
    (266,194,'/data',1,1,0),
    (267,195,'/data',1,1,0),
    (268,196,'/data',1,1,0),
    (269,197,'/data',1,1,0),
    (270,198,'/data',1,1,0),
    (271,199,'/lustre',1,1,0);

INSERT INTO hosts VALUES (1,'lse001',0,1),
    (2,'lse002',0,0),
    (3,'lse003',0,1),
    (4,'lse004',0,1),
    (5,'lse005',0,1),
    (6,'lse006',0,1),
    (7,'lse007',0,1),
    (8,'lse008',0,1),
    (9,'lse009',0,1),
    (10,'lse010',0,1),
    (11,'lse011',0,1),
    (12,'lse012',0,1),
    (13,'lse013',0,1),
    (14,'lse014',0,1),
    (15,'lse015',0,1),
    (16,'lse016',0,1),
    (17,'lse017',0,1),
    (18,'lse018',0,1),
    (19,'lse019',0,0),
    (20,'lse020',0,1),
    (21,'lse021',0,1),
    (22,'lse022',0,1),
    (23,'lse023',0,1),
    (24,'lse024',0,1),
    (25,'lce001',1,0),
    (26,'lce002',1,0),
    (27,'lce003',1,0),
    (28,'lce004',1,0),
    (29,'lce005',1,0),
    (30,'lce006',1,0),
    (31,'lce007',1,0),
    (32,'lce008',1,0),
    (33,'lce009',1,0),
    (34,'lce010',1,0),
    (35,'lce011',1,0),
    (36,'lce012',1,0),
    (37,'lce013',1,0),
    (38,'lce014',1,0),
    (39,'lce015',1,0),
    (40,'lce016',1,0),
    (41,'lce017',1,0),
    (42,'lce018',1,0),
    (43,'lce019',1,0),
    (44,'lce020',1,0),
    (45,'lce021',1,0),
    (46,'lce022',1,0),
    (47,'lce023',1,0),
    (48,'lce024',1,0),
    (49,'lce025',1,0),
    (50,'lce026',1,0),
    (51,'lce027',1,0),
    (52,'lce028',1,0),
    (53,'lce029',1,0),
    (54,'lce030',1,0),
    (55,'lce031',1,0),
    (56,'lce032',1,0),
    (57,'lce033',1,0),
    (58,'lce034',1,0),
    (59,'lce035',1,0),
    (60,'lce036',1,0),
    (61,'lce037',1,0),
    (62,'lce038',1,0),
    (63,'lce039',1,0),
    (64,'lce040',1,0),
    (65,'lce041',1,0),
    (66,'lce042',1,0),
    (67,'lce043',1,0),
    (68,'lce044',1,0),
    (69,'lce045',1,0),
    (70,'lce046',1,0),
    (71,'lce047',1,0),
    (72,'lce048',1,0),
    (73,'lce049',1,0),
    (74,'lce050',1,0),
    (75,'lce051',1,0),
    (76,'lce052',1,0),
    (77,'lce053',1,0),
    (78,'lce054',1,0),
    (79,'lce055',1,0),
    (80,'lce056',1,0),
    (81,'lce057',1,0),
    (82,'lce058',1,0),
    (83,'lce059',1,0),
    (84,'lce060',1,0),
    (85,'lce061',1,0),
    (86,'lce062',1,0),
    (87,'lce063',1,0),
    (88,'lce064',1,0),
    (89,'lce065',1,0),
    (90,'lce066',1,0),
    (91,'lce067',1,0),
    (92,'lce068',1,0),
    (93,'lce069',1,0),
    (94,'lce070',1,0),
    (95,'lce071',1,0),
    (96,'lce072',1,0),
    (97,'lexar001',2,0),
    (98,'lexar002',2,0),
    (99,'locus001',3,0),
    (100,'locus002',3,0),
    (101,'locus003',3,0),
    (102,'locus004',3,0),
    (103,'locus005',3,0),
    (104,'locus006',3,0),
    (105,'locus007',3,0),
    (106,'locus008',3,0),
    (107,'locus009',3,0),
    (108,'locus010',3,0),
    (109,'locus011',3,0),
    (110,'locus012',3,0),
    (111,'locus013',3,0),
    (112,'locus014',3,0),
    (113,'locus015',3,0),
    (114,'locus016',3,0),
    (115,'locus017',3,0),
    (116,'locus018',3,0),
    (117,'locus019',3,0),
    (118,'locus020',3,0),
    (119,'locus021',3,0),
    (120,'locus022',3,0),
    (121,'locus023',3,0),
    (122,'locus024',3,0),
    (123,'locus025',3,0),
    (124,'locus026',3,0),
    (125,'locus027',3,0),
    (126,'locus028',3,0),
    (127,'locus029',3,0),
    (128,'locus030',3,0),
    (129,'locus031',3,0),
    (130,'locus032',3,0),
    (131,'locus033',3,0),
    (132,'locus034',3,0),
    (133,'locus035',3,0),
    (134,'locus036',3,0),
    (135,'locus037',3,0),
    (136,'locus038',3,0),
    (137,'locus039',3,0),
    (138,'locus040',3,0),
    (139,'locus041',3,0),
    (140,'locus042',3,0),
    (141,'locus043',3,0),
    (142,'locus044',3,0),
    (143,'locus045',3,0),
    (144,'locus046',3,0),
    (145,'locus047',3,0),
    (146,'locus048',3,0),
    (147,'locus049',3,0),
    (148,'locus050',3,0),
    (149,'locus051',3,0),
    (150,'locus052',3,0),
    (151,'locus053',3,0),
    (152,'locus054',3,0),
    (153,'locus055',3,0),
    (154,'locus056',3,0),
    (155,'locus057',3,0),
    (156,'locus058',3,0),
    (157,'locus059',3,0),
    (158,'locus060',3,0),
    (159,'locus061',3,0),
    (160,'locus062',3,0),
    (161,'locus063',3,0),
    (162,'locus064',3,0),
    (163,'locus065',3,0),
    (164,'locus066',3,0),
    (165,'locus067',3,0),
    (166,'locus068',3,0),
    (167,'locus069',3,0),
    (168,'locus070',3,0),
    (169,'locus071',3,0),
    (170,'locus072',3,0),
    (171,'locus073',3,0),
    (172,'locus074',3,0),
    (173,'locus075',3,0),
    (174,'locus076',3,0),
    (175,'locus077',3,0),
    (176,'locus078',3,0),
    (177,'locus079',3,0),
    (178,'locus080',3,0),
    (179,'locus081',3,0),
    (180,'locus082',3,0),
    (181,'locus083',3,0),
    (182,'locus084',3,0),
    (183,'locus085',3,0),
    (184,'locus086',3,0),
    (185,'locus087',3,0),
    (186,'locus088',3,0),
    (187,'locus089',3,0),
    (188,'locus090',3,0),
    (189,'locus091',3,0),
    (190,'locus092',3,0),
    (191,'locus093',3,0),
    (192,'locus094',3,0),
    (193,'locus095',3,0),
    (194,'locus096',3,0),
    (195,'locus097',3,0),
    (196,'locus098',3,0),
    (197,'locus099',3,0),
    (198,'locus100',3,0),
    (199,'mcu005',4,0);
commit;

