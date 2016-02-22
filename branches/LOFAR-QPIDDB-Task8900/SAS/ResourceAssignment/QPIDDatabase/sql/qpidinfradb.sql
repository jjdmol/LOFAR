DROP TABLE IF EXISTS exchanges;
DROP TABLE IF EXISTS queues;
DROP TABLE IF EXISTS hosts;
DROP TABLE IF EXISTS persistentexchanges;
DROP TABLE IF EXISTS persistentqueues;
DROP TABLE IF EXISTS queueroutes;
DROP TABLE IF EXISTS exchangeroutes;
DROP TABLE IF EXISTS queuelistener;

CREATE TABLE exchanges(
    exchangeid SERIAL,
    exchangename varchar(512) NOT NULL,
    PRIMARY KEY  (exchangeid)
);
CREATE TABLE queues(
    queueid  SERIAL,
    queuename varchar(512) NOT NULL,
    PRIMARY KEY  (queueid)
);
CREATE TABLE hosts(
    hostid  SERIAL,
    hostname varchar(512) NOT NULL,
    PRIMARY KEY  (hostid)
);

CREATE TABLE persistentexchanges (
    pexid SERIAL,
    eid bigint  NOT NULL,
    hid bigint  NOT NULL,
    PRIMARY KEY  (pexid)
);

CREATE TABLE persistentqueues (
    pquid SERIAL,
    qid bigint  NOT NULL,
    hid bigint  NOT NULL,
    PRIMARY KEY  (pquid)
);

CREATE TABLE queueroutes(
    qrouteid SERIAL,
    fromhost bigint  NOT NULL,
    tohost bigint  NOT NULL,
    qid  bigint  NOT NULL,
    eid  bigint  NOT NULL,
    PRIMARY KEY  (qrouteid)
);
CREATE TABLE exchangeroutes(
    erouteid SERIAL,
    fromhost bigint  NOT NULL,
    tohost bigint  NOT NULL,
    eid bigint  NOT NULL,
    dynamic bool default false,
    routingkey varchar(512) default '#',
    PRIMARY KEY  (erouteid)
);
CREATE TABLE queuelistener(
    qlistenid SERIAL,
    fromhost bigint  NOT NULL,
    eid bigint  NOT NULL,
    qid bigint NOT NULL,
    subject varchar(512) NOT NULL,
    PRIMARY KEY  (qlistenid)
);


INSERT INTO exchanges (exchangename) VALUES 
    ('lofar.ra.command'),
    ('lofar.ra.notification'),
    ('lofar.otdb.command'),
    ('lofar.otdb.notification'),
    ('lofar.sm.command'),
    ('lofar.sm.notification'),
    ('lofar.mom.command'),
    ('lofar.mom.notification');

INSERT INTO queues (queuename) VALUES 
    ('TreeStatus'),
    ('TaskSpecified'),
    ('ResourceAssigner');

INSERT INTO hosts (hostname) VALUES
    ('scu001.control.lofar'),
    ('ccu001.control.lofar'),
    ('head01.control.lofar');


INSERT INTO persistentexchanges (eid,hid) VALUES 
    (1,1),
    (2,1),
    (3,1),
    (4,1),
    (5,1),
    (6,1),
    (7,1),
    (8,1);

insert INTO persistentqueues (qid,hid) VALUES
    (1,1),
    (2,1),
    (3,1);

commit;
