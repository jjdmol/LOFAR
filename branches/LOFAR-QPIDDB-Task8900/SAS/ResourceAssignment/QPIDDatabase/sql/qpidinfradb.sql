DROP TABLE IF EXISTS exchanges;
DROP TABLE IF EXISTS queues;
DROP TABLE IF EXISTS hosts;
DROP TABLE IF EXISTS persistentexchanges;
DROP TABLE IF EXISTS persistentqueues;
DROP TABLE IF EXISTS queueroutes;
DROP TABLE IF EXISTS exchangeroutes;
DROP TABLE IF EXISTS queuelistener;

CREATE TABLE exchanges(
    exchangeid bigint  NOT NULL,
    exchangename varchar(512) NOT NULL
);
CREATE TABLE queues(
    queueid  bigint  NOT NULL,
    queuename varchar(512) NOT NULL
);
CREATE TABLE hosts(
    hostid  bigint  NOT NULL,
    hostname varchar(512) NOT NULL
);

CREATE TABLE persistentexchanges (
    pexid bigint  NOT NULL,
    eid bigint  NOT NULL,
    hid bigint  NOT NULL
);

CREATE TABLE persistentqueues (
    pquid bigint  NOT NULL,
    qid bigint  NOT NULL,
    hid bigint  NOT NULL
);

CREATE TABLE queueroutes(
    fromhost bigint  NOT NULL,
    tohost bigint  NOT NULL,
    qid  bigint  NOT NULL
);
CREATE TABLE exchangeroutes(
    fromhost bigint  NOT NULL,
    tohost bigint  NOT NULL,
    eid bigint  NOT NULL
);
CREATE TABLE queuelistener(
    fromhost bigint  NOT NULL,
    eid bigint  NOT NULL,
    qid bigint NOT NULL,
    subject varchar(512) NOT NULL
);


INSERT INTO exchanges VALUES 
    (1,'lofar.ra.command'),
    (2,'lofar.ra.notification'),
    (3,'lofar.otdb.command'),
    (4,'lofar.otdb.notification'),
    (5,'lofar.sm.command'),
    (6,'lofar.sm.notification'),
    (7,'lofar.mom.command'),
    (8,'lofar.mom.notification');

INSERT INTO queues VALUES 
    (1,'TreeStatus'),
    (2,'TaskSpecified'),
    (3,'ResourceAssigner');

INSERT INTO hosts VALUES
    (1,'scu001.control.lofar'),
    (2,'ccu001.control.lofar'),
    (3,'head01.control.lofar');


INSERT INTO persistentexchanges VALUES 
    (1,1,1),
    (2,2,1),
    (3,3,1),
    (4,4,1),
    (5,5,1),
    (6,6,1),
    (7,7,1),
    (8,8,1);

insert INTO persistentqueues VALUES
    (1,1,1),
    (2,2,1),
    (3,3,1);

commit;
