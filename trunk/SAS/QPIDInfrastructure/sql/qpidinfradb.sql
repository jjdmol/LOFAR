-- Usage:
--
--   psql --command="CREATE DATABASE qpidinfra"
--   psql --dbname=qpidinfra --file=qpidinfradb.sql

DROP TABLE IF EXISTS exchanges CASCADE;
DROP TABLE IF EXISTS queues CASCADE;
DROP TABLE IF EXISTS hosts CASCADE;
DROP TABLE IF EXISTS persistentexchanges CASCADE;
DROP TABLE IF EXISTS persistentqueues CASCADE;
DROP TABLE IF EXISTS queueroutes CASCADE;
DROP TABLE IF EXISTS exchangeroutes CASCADE;
DROP TABLE IF EXISTS queuelistener CASCADE;

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
    eid bigint  references exchanges(exchangeid) ON DELETE CASCADE,
    hid bigint  references hosts(hostid) ON DELETE CASCADE,
    PRIMARY KEY  (pexid)
);

CREATE TABLE persistentqueues (
    pquid SERIAL,
    qid bigint  references queues(queueid) ON DELETE CASCADE,
    hid bigint  references hosts(hostid) ON DELETE CASCADE,
    PRIMARY KEY  (pquid)
);

CREATE TABLE queueroutes(
    qrouteid SERIAL,
    fromhost bigint  references hosts(hostid) ON DELETE CASCADE,
    tohost bigint  references hosts(hostid) ON DELETE CASCADE,
    qid  bigint  references queues(queueid) ON DELETE CASCADE,
    eid  bigint  references exchanges(exchangeid) ON DELETE CASCADE,
    PRIMARY KEY  (qrouteid)
);

CREATE TABLE exchangeroutes(
    erouteid SERIAL,
    fromhost bigint  references hosts(hostid) ON DELETE CASCADE,
    tohost bigint  references hosts(hostid) ON DELETE CASCADE,
    eid bigint references exchanges(exchangeid) ON DELETE CASCADE,
    dynamic bool default false,
    routingkey varchar(512) default '#',
    PRIMARY KEY  (erouteid)
);

CREATE TABLE queuelistener(
    qlistenid SERIAL,
    fromhost bigint references hosts(hostid) ON DELETE CASCADE,
    eid bigint  references exchanges(exchangeid) ON DELETE CASCADE,
    qid bigint references queues(queueid) ON DELETE CASCADE,
    subject varchar(512) NOT NULL,
    PRIMARY KEY  (qlistenid)
);

COMMIT;
